#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>

// Structure to store edgel information
struct Edgel 
{
    cv::Point2f midpoint;         // Midpoint of the edge
    cv::Point2f endpoint1;        // First endpoint of the zero crossing
    cv::Point2f endpoint2;        // Second endpoint of the zero crossing
    float gradientMagnitude;      // Magnitude of the gradient at the edgel
    float orientation;            // Orientation of the gradient or the edge
};

int main() 
{
    // Load image from the specified path
    cv::Mat image = cv::imread("C:\\Users\\aaron\\Desktop\\Baby_Penny.jpeg");

    // Check if the image was loaded successfully
    if (image.empty()) 
    {
        std::cerr << "Error loading image!" << std::endl;
        return -1; // Exit the program if the image is not found
    }

    // Convert the image to grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Step 1: Blur the input image to reduce noise
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.0);

    // Step 2: Construct a Gaussian Pyramid
    std::vector<cv::Mat> gaussianPyramid;     // Vector to store pyramid levels
    gaussianPyramid.push_back(blurred);       // First level is the blurred image
    for (int i = 0; i < 3; ++i)               // Create three additional pyramid levels
    {   
        cv::Mat temp;
        cv::pyrDown(gaussianPyramid[i], temp); // Downsample the current level
        gaussianPyramid.push_back(temp);       // Add the downsampled image to the pyramid
    }

    // Step 3: Subtract interpolated coarser-level pyramid image from the original resolution blurred image
    cv::Mat interpolated, S;
    cv::pyrUp(gaussianPyramid[1], interpolated, blurred.size()); // Upsample the first level of the pyramid
    S = blurred - interpolated;                                  // Subtract the upsampled image from the original blurred image

    // Step 4: Detect zero crossings and Step 5: Store edgels
    std::vector<Edgel> edgels;                                   // Vector to store detected edgels
    cv::Mat laplacian;
    cv::Laplacian(S, laplacian, CV_64F);                         // Compute the Laplacian to find zero crossings

    // Iterate through each quad of pixels in the Laplacian image
    for (int i = 0; i < laplacian.rows - 1; ++i) 
    {
        for (int j = 0; j < laplacian.cols - 1; ++j) 
        {
            // Get pixel values for the quad
            double p1 = laplacian.at<double>(i, j);
            double p2 = laplacian.at<double>(i + 1, j);
            double p3 = laplacian.at<double>(i, j + 1);
            double p4 = laplacian.at<double>(i + 1, j + 1);

            int zeroCrossings = 0;                  // Counter for zero crossings
            cv::Point2f zeroCrossing1, zeroCrossing2; // Points to store zero crossing locations

            // Check for zero crossings along the four edges of the quad
            if ((p1 > 0 && p2 < 0) || (p1 < 0 && p2 > 0)) 
            {
                zeroCrossing1 = cv::Point2f(i + 0.5, j); // Zero crossing along vertical edge (i, j) to (i+1, j)
                zeroCrossings++;
            }
            if ((p1 > 0 && p3 < 0) || (p1 < 0 && p3 > 0)) 
            {
                if (zeroCrossings == 0) zeroCrossing1 = cv::Point2f(i, j + 0.5); // Zero crossing along horizontal edge (i, j) to (i, j+1)
                else zeroCrossing2 = cv::Point2f(i, j + 0.5); // Assign to the second crossing if one has already been found
                zeroCrossings++;
            }
            if ((p2 > 0 && p4 < 0) || (p2 < 0 && p4 > 0)) 
            {
                if (zeroCrossings == 0) zeroCrossing1 = cv::Point2f(i + 1, j + 0.5); // Zero crossing along horizontal edge (i+1, j) to (i+1, j+1)
                else zeroCrossing2 = cv::Point2f(i + 1, j + 0.5); // Assign to the second crossing if one has already been found
                zeroCrossings++;
            }
            if ((p3 > 0 && p4 < 0) || (p3 < 0 && p4 > 0)) 
            {
                if (zeroCrossings == 0) zeroCrossing1 = cv::Point2f(i + 0.5, j + 1); // Zero crossing along vertical edge (i, j+1) to (i+1, j+1)
                else zeroCrossing2 = cv::Point2f(i + 0.5, j + 1); // Assign to the second crossing if one has already been found
                zeroCrossings++;
            }

            // Step 6: If exactly two zero crossings, compute their locations and store in edgel structure
            if (zeroCrossings == 2) 
            {
                cv::Point2f midpoint = (zeroCrossing1 + zeroCrossing2) * 0.5f; // Calculate midpoint between the two crossings
                cv::Point2f gradient = zeroCrossing2 - zeroCrossing1;          // Calculate gradient vector
                float gradientMagnitude = cv::norm(gradient);                  // Compute magnitude of the gradient
                float orientation = atan2(gradient.y, gradient.x);             // Compute orientation of the gradient

                // Create an edgel and add it to the list
                Edgel edgel = { midpoint, zeroCrossing1, zeroCrossing2, gradientMagnitude, orientation };
                edgels.push_back(edgel);
            }
        }
    }

    // Step 7: Display the original grayscale image and the computed S(x) image
    cv::imshow("Original Grayscale Image", gray);
    cv::imshow("S(x) Image", S);

    // Visualize edgels (optional)
    for (const auto& edgel : edgels) 
    {
        cv::circle(S, edgel.midpoint, 2, cv::Scalar(255), -1); // Draw a small circle at the midpoint of the edgel
        cv::line(S, edgel.endpoint1, edgel.endpoint2, cv::Scalar(255)); // Draw a line between the endpoints of the edgel
    }
    cv::imshow("Edgels", S); // Display the image with the visualized edgels

    cv::waitKey(0); // Wait for a key press before closing the windows
}