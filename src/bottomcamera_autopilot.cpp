#include "ros/ros.h"
#include "ros/package.h"
#include <ros/node_handle.h>
#include <cv_bridge/cv_bridge.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/Vector3.h>
#include <geometry_msgs/Twist.h>
#include "std_msgs/String.h"
#include <ar_pose/ARMarker.h>
#include <tf2/transform_datatypes.h>
#include <tf2_ros/transform_listener.h>
#include <boost/lexical_cast.hpp>



const double PI = 3.141593;
unsigned int ros_header_timestamp_base = 0;

using namespace std;


int main ( int argc, char **argv )
{
        ros::init ( argc, argv,"pose_subscriber" );
        ROS_INFO ( "TF subscriber correctly running..." );
        ros::NodeHandle nh;
        tf2_ros::Buffer tfBuffer;
        tf2_ros::TransformListener tfListener ( tfBuffer );
        ros::Rate rate ( 5.0 );
        ros::Publisher cmd_pub = nh.advertise<std_msgs::String> ( "/uga_tum_ardrone/com",100 );

        int count = 1;
        // Save the translation and rotational offsets on three axis
        float linear_offset_X, linear_offset_Y, linear_offset_Z;
        float rotational_offset_Z;
        linear_offset_X = linear_offset_Y = linear_offset_Z = rotational_offset_Z = 0;

        float target_X, target_Y,target_Z, target_yaw;
        target_X = 0.1;
        target_Y = 0.1;
        target_Z = 1.0;
        target_yaw = 5.0;

        while ( nh.ok() ) {
                geometry_msgs::TransformStamped transformStamped;
                geometry_msgs::Vector3 errors_msg;
                try {
                        transformStamped = tfBuffer.lookupTransform ( "ar_marker","ardrone_base_bottomcam",ros::Time ( 0 ) );
                        //transformStamped = tfBuffer.lookupTransform( "ar_marker", "base_link", ros::Time(0));
                } catch ( tf2::TransformException &ex ) {
                        ROS_WARN ( "%s",ex.what() );
                        ros::Duration ( 1.0 ).sleep();
                        continue;
                }

                /*
                * Shut down the controller if the marker is outside the camera's FOV
                * NOT WORKING
                * TODO : fix it!
                if( (transformStamped.transform.translation.x == transformStamped.transform.translation.y == transformStamped.transform.translation.z == 0)){
                	ROS_WARN ( "Marker Lost or not found" );
                	ros::Duration ( 1.0 ).sleep();
                         ros::shutdown();
                }*/

                /* TODO
                 * Fix axis correspondences
                 */


                // Adjust distance on the global X axis - z for marker and camera
                // Adjust distance on the global y axis - x for marker and camera
                // Adjust distance on the global z axis - y for marker and camera
                if ( transformStamped.transform.translation.y > 0 ) {
                        linear_offset_X = - transformStamped.transform.translation.x;
                        linear_offset_Y = - transformStamped.transform.translation.y;
                        linear_offset_Z = - abs(transformStamped.transform.translation.z);
                        float tmp_rotation =  transformStamped.transform.rotation.z;
                        rotational_offset_Z = ( tmp_rotation * 180 / PI );
                } else {
                        linear_offset_X = - transformStamped.transform.translation.x; 
                        linear_offset_Y = - transformStamped.transform.translation.y; 
                        linear_offset_Z = - abs(transformStamped.transform.translation.z);
                        float tmp_rotation = - transformStamped.transform.rotation.z;
                        rotational_offset_Z = ( tmp_rotation * 180 / PI );
                }

                cout << "TF : " << transformStamped.transform.translation.x << " " << transformStamped.transform.translation.y << " " << transformStamped.transform.translation.z << " with a yaw of " << transformStamped.transform.rotation.z << endl;
                std_msgs::String clear, autoinit,takeoff,goTo, moveBy ,land, reference, maxControl, initialReachDist, stayWithinDist, stayTime;

                if ( abs ( linear_offset_X ) < target_X ) {
                        linear_offset_X = 0;
                }

                if ( abs ( linear_offset_Y ) < target_Y ) {
                        linear_offset_Y = 0;
                }

                if ( abs ( linear_offset_Z ) < target_Z ) {
                        linear_offset_Z = 0;
                }

                if ( abs ( rotational_offset_Z ) < target_yaw ) {
                        rotational_offset_Z = 0;
                }

                clear.data = "c clearCommands";

                autoinit.data = "c autoInit 500 800";
                reference.data = "setReference $POSE$";
                maxControl.data = "setMaxControl 1";
                initialReachDist.data = "setInitialReachDist 0.1";
                stayWithinDist.data = "setStayWithinDist 0.1";
                stayTime.data = "setStayTime 0.5";

                takeoff.data = "c takeoff";


                //goTo.data = "c goto " + boost::lexical_cast<std::string> ( linear_offset_X ) + " " + boost::lexical_cast<std::string> ( linear_offset_Y ) + " " +
                //            boost::lexical_cast<std::string> ( linear_offset_Z ) + " " + boost::lexical_cast<std::string> ( rotational_offset_Z ) ;

                moveBy.data = "c moveByRel " + boost::lexical_cast<std::string> ( linear_offset_X ) + " " + boost::lexical_cast<std::string> ( linear_offset_Y ) + " " +
                              boost::lexical_cast<std::string> ( linear_offset_Z ) + " " + boost::lexical_cast<std::string> ( rotational_offset_Z ) ;
                land.data = "c land";



                if (  abs ( linear_offset_X ) > target_X || abs ( linear_offset_Y ) > target_Y || abs ( linear_offset_Z ) > target_Z || abs ( rotational_offset_Z ) > target_yaw  ) {

                        cmd_pub.publish<> ( clear );
                        cmd_pub.publish<> ( autoinit );
                        cmd_pub.publish<> ( reference );
                        cmd_pub.publish<> ( maxControl );
                        cmd_pub.publish<> ( initialReachDist );
                        cmd_pub.publish<> ( stayWithinDist );
                        cmd_pub.publish<> ( stayTime );
                        //cmd_pub.publish<> ( takeoff );	// mandatory for taking off (despite in the gui the drone takes off also using only the autoInit)
                        cmd_pub.publish<> ( moveBy );
                        
                        count ++;
                        cout << "Autopilot : " << linear_offset_X << " " << linear_offset_Y << " " << linear_offset_Z << " " << rotational_offset_Z << endl;
			cout << moveBy.data << endl;
                        cout <<  endl;
                        ros::Duration ( 1.0 ).sleep();
                } else {
                        ROS_INFO ( "Destination reached" );
			cmd_pub.publish<> ( land );
                        count ++;
                        ros::shutdown();
                }






                rate.sleep();
        }



        ros::spin();
        return 0;

}