#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    // ROS_INFO_STREAM("Moving the arm to the center");

    // Request lin_x, ang_z
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the safe_move service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service drive to target");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    int white_pixel = 255;
    bool found_ball = false;
    int screen_split; 

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    for (int i = 0; i < img.height; ++i)
    {
        for (int j = 0; j < img.step; j+=3)
        {
            int r = img.data[i*img.step + j];
            int g = img.data[i*img.step + j+1];
            int b = img.data[i*img.step + j+2];
            if ( r + g + b == 255*3 )
            {
                ROS_INFO_STREAM("found at: [ " + std::to_string(i) + ", " + std::to_string(int(j/3)) + " ]");
                found_ball = true;
                screen_split = j / img.width ;
                break;
            }
            
        }
    }

    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    ROS_INFO_STREAM("SCREEN SPLIT: " + std::to_string(screen_split));
    if (found_ball)
    {
        // Then, identify if this pixel falls in the left, mid, or right side of the image
        if (screen_split == 0 ) // in left
        {
                    drive_robot(0.2, 0.5);
                    ROS_INFO_STREAM("MOVED LEFT");
        }
        else if (screen_split == 1) // in center
        {
                    drive_robot(0.2, 0.0);
                    ROS_INFO_STREAM("MOVED FORWARD");
        }
        else // in right
        {
            drive_robot(0.2, -0.5);
            ROS_INFO_STREAM("MOVED RIGHT");
        }
    }
    else
    {
        drive_robot(0, 0); 
        ROS_INFO_STREAM("DO NOTHING");
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 20, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}