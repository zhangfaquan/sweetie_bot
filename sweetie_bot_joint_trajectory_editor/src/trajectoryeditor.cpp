#include "trajectoryeditor.h"
#include "ui_Editor.h"

#include <QStandardItemModel>
#include <QMessageBox>


TrajectoryEditor::TrajectoryEditor(int argc, char *argv[], ros::NodeHandle node, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrajectoryEditor),
    node_(node),
    loader_(new sweetie_bot::tools::ParamMsgLoader<control_msgs::FollowJointTrajectoryGoal>(node))
{
    ui->setupUi(this);
    updateParamList();
	ui->comboBox->setCurrentText("default");

    ui->timeIncrementSpinBox->setValue(0.0);

    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(rosSpin()));

    timer->start(50);

    sub_real_ = node.subscribe<sensor_msgs::JointState>("/trajectory_editor/joint_trajectory_editor/joints_real", 1000, &TrajectoryEditor::jointsRealCallback, this);
    sub_virtual_ = node.subscribe<sensor_msgs::JointState>("/sweetie_bot/joint_trajectory_editor/joint_virtual", 1000, &TrajectoryEditor::jointsVirtualCallback, this);
	pub_joints_virtual_set = node.advertise<sensor_msgs::JointState>("/sweetie_bot/joint_trajectory_editor/joints_virtual_set", 1);
	pub_joints_marker_set = node.advertise<sensor_msgs::JointState>("/sweetie_bot/joint_trajectory_editor/joints_marker_set", 1);

    client_virtual = new Client("/sweetie_bot/motion/controller/joint_trajectory", true);
    client_real    = new Client("/sweetie_bot/motion/controller/follow_trajectory", true);
    //client_virtual->waitForServer();
    //state_ = boost::make_shared<actionlib::SimpleClientGoalState>(client->getState());
    //Client::ResultConstPtr result = *client->getResult();


    control_msgs::FollowJointTrajectoryGoal sd = loader_->getParam( trajectories_param_name + "/default");
    ROS_INFO_STREAM( "\n" << sd );
    joint_trajectory_data_ = new sweetie_bot::interface::JointTrajectoryData(sd);

    joint_list_table_view_ = new sweetie_bot::interface::JointListTableView(parent, *joint_trajectory_data_);
    joint_trajectory_point_table_view_ = new sweetie_bot::interface::JointTrajectoryPointTableView(parent, *joint_trajectory_data_);

    bootstrap();
}

TrajectoryEditor::~TrajectoryEditor()
{
    delete ui;
}

void TrajectoryEditor::jointsRealCallback(const sensor_msgs::JointState::ConstPtr& msg)
{
  joint_state_real_ = *msg;
  if(!ui->addRealPoseButton->isEnabled())
    ui->addRealPoseButton->setEnabled(true);
}

void TrajectoryEditor::jointsVirtualCallback(const sensor_msgs::JointState::ConstPtr& msg)
{
  joint_state_virtual_ = *msg;
  if(!ui->addVirtualPoseButton->isEnabled())
    ui->addVirtualPoseButton->setEnabled(true);
}

void TrajectoryEditor::updateParamList()
{
    QString tmp = ui->comboBox->currentText();
    std::vector<std::string> trajectory_names = loader_->getNames(trajectories_param_name);
    ui->comboBox->clear();
    for(auto &tname: trajectory_names)
    {
        ui->comboBox->addItem(QString::fromStdString(tname));
    }
    if(tmp != "") ui->comboBox->setCurrentText(tmp);
}

void TrajectoryEditor::rosSpin()
{
    ros::spinOnce();
}


void TrajectoryEditor::bootstrap()
{
  ui->pointsTableView->setModel(joint_trajectory_point_table_view_);
  ui->jointsTableView->setModel(joint_list_table_view_);
}

void TrajectoryEditor::on_loadTrajectoryButton_clicked()
{
  control_msgs::FollowJointTrajectoryGoal msg;
  bool param_ok = loader_->getParam(trajectories_param_name + "/" + ui->comboBox->currentText().toStdString(), msg);
  joint_trajectory_data_->loadFromMsg( msg );
  if(!param_ok) {
	if(ui->addRealPoseButton->isEnabled())
	{
	  for(auto &name: joint_state_real_.name){
		joint_trajectory_data_->addJoint(name, 0.0, 0.0);
	  }
	} else if (ui->addVirtualPoseButton->isEnabled()){
	  for(auto &name: joint_state_virtual_.name){
		joint_trajectory_data_->addJoint(name, 0.0, 0.0);
	  }
	}
  }
  joint_list_table_view_->reReadData();
  joint_trajectory_point_table_view_->reReadData();
  ui->goalTimeToleranceSpinBox->setValue(joint_trajectory_data_->getGoalTimeTolerance());
}

void TrajectoryEditor::on_turnAllServoOnButton_clicked()
{

  if (ui->turnAllServoOnButton->text() == "Turn all servos on")
  {
    ROS_INFO("Command to turn on servos");
    ui->turnAllServoOnButton->setText("Turn all servos off");
  }
  else
  {
    ROS_INFO("Command to turn off servos");
    ui->turnAllServoOnButton->setText("Turn all servos on");
  }
}

void TrajectoryEditor::on_turnAllTrajectoryServosButton_clicked()
{
  ROS_INFO("Command to switch trajectory servos");
}

void TrajectoryEditor::on_turnAllSelectedServosButton_clicked()
{
  ROS_INFO("Command to switch selected servos");
}

void TrajectoryEditor::on_addVirtualPoseButton_clicked()
{
	ROS_INFO_STREAM("\n" << joint_state_virtual_ );
	joint_trajectory_data_->addPoint(joint_state_virtual_, ui->timeIncrementSpinBox->value());
	joint_trajectory_point_table_view_->reReadData();
}

void TrajectoryEditor::on_addRealPoseButton_clicked()
{
    joint_trajectory_data_->addPoint(joint_state_real_, ui->timeIncrementSpinBox->value());
    joint_trajectory_point_table_view_->reReadData();
}

void TrajectoryEditor::on_saveTrajectoryButton_clicked()
{
    loader_->setParam(trajectories_param_name + "/" + ui->comboBox->currentText().toStdString(), joint_trajectory_data_->getTrajectoryMsg());
    std::string cmd = "rosparam dump `rospack find sweetie_bot_joint_trajectory_editor`/launch/trajectories.yaml " + trajectories_param_name;
    system( cmd.c_str() );
    updateParamList();
}

void TrajectoryEditor::on_deletePoseButton_clicked()
{
    joint_trajectory_point_table_view_->removeRow(ui->pointsTableView->selectionModel()->currentIndex().row(), QModelIndex());
    joint_trajectory_point_table_view_->reReadData();
}

void TrajectoryEditor::executeActionCallback(const actionlib::SimpleClientGoalState& state, const control_msgs::FollowJointTrajectoryActionResultConstPtr& result)
{
  ROS_INFO("TrajectoryEditor::executeActionCallback");
}

void TrajectoryEditor::on_executeButton_clicked()
{
  bool reverse = ui->backwardCheckBox->isChecked();
  control_msgs::FollowJointTrajectoryGoal goal = joint_trajectory_data_->getTrajectoryMsg(reverse);
  if(ui->virtualCheckBox->isChecked())
  {
    client_virtual->waitForServer();
    actionlib::SimpleClientGoalState state = client_virtual->sendGoalAndWait( goal );
    ROS_INFO_STREAM("\n" << goal);
    ROS_INFO("%s", state.toString().c_str());
    ui->statusLabel->setText("Status: " + QString::fromStdString(state.toString()) );
  }else{
    client_real->waitForServer();
    actionlib::SimpleClientGoalState state = client_real->sendGoalAndWait( goal );
    ROS_INFO_STREAM("\n" << goal);
    ROS_INFO("%s", state.toString().c_str());
    ui->statusLabel->setText("Status: " + QString::fromStdString(state.toString()) );
  }
  // TODO callback does not compile T_T
  //client->sendGoal(joint_trajectory_data_->follow_joint_trajectory_goal_, boost::bind(&TrajectoryEditor::executeActionCallback, this, _1, _2));
  //, Client::SimpleActiveCallback(), Client::SimpleFeedbackCallback());
}

void TrajectoryEditor::on_jointsTableView_clicked(const QModelIndex &index)
{
  std::string joint_name = joint_list_table_view_->data(index).toString().toStdString();
  ui->jointNameEditBox->setText(joint_list_table_view_->data(index).toString());
  ui->pathToleranceSpinBox->setValue(joint_trajectory_data_->getPathTolerance(joint_name));
  ui->goalToleranceSpinBox->setValue(joint_trajectory_data_->getGoalTolerance(joint_name));
}

void TrajectoryEditor::on_addButton_clicked()
{
    joint_trajectory_data_->addJoint(ui->jointNameEditBox->text().toStdString(), ui->pathToleranceSpinBox->value(), ui->goalToleranceSpinBox->value());
    joint_list_table_view_->reReadData();
}

void TrajectoryEditor::on_applyButton_clicked()
{
    joint_trajectory_data_->setPathTolerance(ui->jointNameEditBox->text().toStdString(), ui->pathToleranceSpinBox->value());
    joint_trajectory_data_->setGoalTolerance(ui->jointNameEditBox->text().toStdString(), ui->goalToleranceSpinBox->value());
}

void TrajectoryEditor::on_delButton_clicked()
{
    if(ui->jointsTableView->selectionModel()->selection().indexes().size() > 0)
    {
      int row = ui->jointsTableView->selectionModel()->currentIndex().row();
      qDebug( "%s", joint_trajectory_data_->getJointName(row).c_str() );
      //joint_list_table_view_->se
      //ROS_INFO(selectedItems())
      joint_trajectory_data_->removeJoint(joint_trajectory_data_->getJointName(row));
      joint_list_table_view_->reReadData();
    }
}

void TrajectoryEditor::on_delTrajectoryButton_clicked()
{
  node_.deleteParam(trajectories_param_name + "/" + ui->comboBox->currentText().toStdString());
  updateParamList();
  ui->comboBox->setCurrentText("default");
}


void TrajectoryEditor::on_goalTimeToleranceSpinBox_valueChanged(double arg1)
{
    joint_trajectory_data_->setGoalTimeTolerance(arg1);
}

void TrajectoryEditor::on_setVirtualPoseButton_clicked()
{
	sensor_msgs::JointState msg = joint_trajectory_data_->getPoint(ui->pointsTableView->selectionModel()->currentIndex().row());
	ROS_INFO_STREAM("\n" << msg);
	pub_joints_virtual_set.publish(msg);
}

void TrajectoryEditor::on_pointsTableView_clicked(const QModelIndex &index)
{
	if(index.isValid()){
	  sensor_msgs::JointState msg = joint_trajectory_data_->getPoint(index.row());
	  ROS_INFO_STREAM("\n" << msg);
	  pub_joints_marker_set.publish(msg);
	}
}

void TrajectoryEditor::on_pointsTableView_doubleClicked(const QModelIndex &index)
{

}

void TrajectoryEditor::on_virtualCheckBox_clicked(bool checked)
{

}
