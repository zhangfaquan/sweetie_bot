cmake_minimum_required(VERSION 2.8.3)
project(sweetie_bot_openmv)

find_package(catkin REQUIRED COMPONENTS rospy geometry_msgs sweetie_bot_text_msgs)

catkin_python_setup()

catkin_package(CATKIN_DEPENDS rospy geometry_msgs sweetie_bot_text_msgs)

catkin_install_python(PROGRAMS scripts/openmv_node
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

install(DIRECTORY launch/ openmv/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
