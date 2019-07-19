#ifndef POSE_MARKER_HPP
#define POSE_MARKER_HPP

#include <tf/tf.h>
#include <tf2_ros/transform_listener.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Twist.h>

#include <interactive_markers/interactive_marker_server.h>
#include <interactive_markers/menu_handler.h>

using interactive_markers::MenuHandler;

namespace sweetie_bot {
namespace hmi {


class PoseMarker {
public:
  PoseMarker(std::shared_ptr<interactive_markers::InteractiveMarkerServer> server,
             const std::string& name,
             double scale = 1.0,
             const std::vector<std::string>& frames = { "bone15", "bone25", "bone35", "bone45", "bone55", "base_link" },
             const std::string& marker_home_frame = "",
             double normalized_z_level = 0.0
            )
    : server(server),
      tf_listener( new tf2_ros::TransformListener(tf_buffer) ),
      name(name + "_pose_marker"),
      scale(scale),
      frames(frames),
      marker_home_frame(marker_home_frame),
      normalized_z_level(normalized_z_level)
  {
  }

  virtual ~PoseMarker() = 0;

protected:
  void processNormalize( const visualization_msgs::InteractiveMarkerFeedbackConstPtr& feedback, const ros::Publisher& pose_pub, bool publish_pose);
  void processMoveToFrame( const visualization_msgs::InteractiveMarkerFeedbackConstPtr& feedback );

  void makeInteractiveMarker(visualization_msgs::Marker (*makeMarkerBody)(double scale),
                             const MenuHandler::FeedbackCallback& processFeedback,
                             bool is6DOF = true);

  virtual void makeMenu() = 0;

  void moveToFrame(const std::string& frame);
  void normalize(geometry_msgs::PoseStamped pose_stamped);

protected:

  // Node handle
	ros::NodeHandle node_handle;

  // COMPONENT INTERFACE

  // CONNECTIONS
  // tf listener
  tf2_ros::Buffer tf_buffer;
  std::unique_ptr<tf2_ros::TransformListener> tf_listener;

  // PARAMETERS
  // marker name
  std::string name;
  // marker sacle parameter
  double scale;
  // frame list: user can place marker to any of provided frames using menu
  std::vector<std::string> frames;
  // Home frame to place marker on start operation. Leave empty to ???
  std::string marker_home_frame;
  // Basic z level. When `Normilize pose` command is executed the marker is placed parallel Oxy plane on normalized_z_level heigh over it.
  double normalized_z_level;

  // COMPONENT STATE
  // interactive marker server
  std::shared_ptr<interactive_markers::InteractiveMarkerServer> server;
  // menu
  MenuHandler menu_handler;
  // menu index
  MenuHandler::EntryHandle normalize_pose_entry;
};

} // namespace hmi
} // namespace sweetie_bot

#endif /*POSE_MARKER_HPP*/
