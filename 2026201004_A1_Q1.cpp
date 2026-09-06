#include <opencv2/opencv.hpp>
#include <cstdio>
#include <cstdlib>
#include <cmath>

inline int idx3(int y, int x, int c, int w) { return (y * w + x) * 3 + c; }
inline int idx1(int y, int x, int w) { return y * w + x; }

// Convert cv::Mat directly into C-style heap buffer
unsigned char* mat_to_buf(const cv::Mat& img, int h, int w) {
    unsigned char* buf = (unsigned char*)malloc(h * w * 3 * sizeof(unsigned char));
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            cv::Vec3b px = img.at<cv::Vec3b>(y, x);
            buf[idx3(y, x, 0, w)] = px[2]; // R
            buf[idx3(y, x, 1, w)] = px[1]; // G
            buf[idx3(y, x, 2, w)] = px[0]; // B
        }
    }
    return buf;
}

// Convert raw buffer back to cv::Mat
cv::Mat buf_to_mat(unsigned char* buf, int h, int w) {
    cv::Mat img(h, w, CV_8UC3);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            cv::Vec3b px;
            px[2] = buf[idx3(y, x, 0, w)]; // R
            px[1] = buf[idx3(y, x, 1, w)]; // G
            px[0] = buf[idx3(y, x, 2, w)]; // B
            img.at<cv::Vec3b>(y, x) = px;
        }
    }
    return img;
}

// Computes energy with clamped borders (prevents border wrap-around artifacts)
double* compute_energy(unsigned char* buf, int h, int w) {
    double* energy = (double*)malloc(h * w * sizeof(double));
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int xl = (x - 1 < 0) ? 0 : x - 1;
            int xr = (x + 1 >= w) ? w - 1 : x + 1;
            int yu = (y - 1 < 0) ? 0 : y - 1;
            int yd = (y + 1 >= h) ? h - 1 : y + 1;

            double dxr = (double)buf[idx3(y, xr, 0, w)] - buf[idx3(y, xl, 0, w)];
            double dxg = (double)buf[idx3(y, xr, 1, w)] - buf[idx3(y, xl, 1, w)];
            double dxb = (double)buf[idx3(y, xr, 2, w)] - buf[idx3(y, xl, 2, w)];
            double dx2 = dxr * dxr + dxg * dxg + dxb * dxb;

            double dyr = (double)buf[idx3(yd, x, 0, w)] - buf[idx3(yu, x, 0, w)];
            double dyg = (double)buf[idx3(yd, x, 1, w)] - buf[idx3(yu, x, 1, w)];
            double dyb = (double)buf[idx3(yd, x, 2, w)] - buf[idx3(yu, x, 2, w)];
            double dy2 = dyr * dyr + dyg * dyg + dyb * dyb;

            energy[idx1(y, x, w)] = sqrt(dx2 + dy2);
        }
    }
    return energy;
}

// Finds vertical seam using Dynamic Programming
int* find_vertical_seam(double* energy, int h, int w) {
    double* cost = (double*)malloc(h * w * sizeof(double));
    int* choice = (int*)malloc(h * w * sizeof(int));

    for (int x = 0; x < w; x++) cost[idx1(0, x, w)] = energy[idx1(0, x, w)];

    for (int y = 1; y < h; y++) {
        for (int x = 0; x < w; x++) {
            double best = cost[idx1(y - 1, x, w)];
            int bestc = 0;
            if (x - 1 >= 0 && cost[idx1(y - 1, x - 1, w)] < best) {
                best = cost[idx1(y - 1, x - 1, w)];
                bestc = -1;
            }
            if (x + 1 < w && cost[idx1(y - 1, x + 1, w)] < best) {
                best = cost[idx1(y - 1, x + 1, w)];
                bestc = 1;
            }
            cost[idx1(y, x, w)] = energy[idx1(y, x, w)] + best;
            choice[idx1(y, x, w)] = bestc;
        }
    }

    int endx = 0;
    double bestend = cost[idx1(h - 1, 0, w)];
    for (int x = 1; x < w; x++) {
        if (cost[idx1(h - 1, x, w)] < bestend) {
            bestend = cost[idx1(h - 1, x, w)];
            endx = x;
        }
    }

    int* seam = (int*)malloc(h * sizeof(int));
    seam[h - 1] = endx;
    for (int y = h - 1; y > 0; y--) {
        seam[y - 1] = seam[y] + choice[idx1(y, seam[y], w)];
    }

    free(cost);
    free(choice);
    return seam;
}

// Displays red seam on screen and saves the visualization image to disk
void visualize_seam(unsigned char* buf, int h, int w, int* seam, const char* window_name) {
    cv::Mat seam_img = buf_to_mat(buf, h, w);

    // Draw bright red pixels along the seam
    for (int y = 0; y < h; y++) {
        int x = seam[y];
        seam_img.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 255); // BGR: Red
    }

    // 1. Display on screen (with delay to make it visible)
    cv::imshow(window_name, seam_img);
    cv::waitKey(20); // 20ms pause per seam

    // 2. Save the very first seam image as seam_visualized.jpg (Figure 2 requirement)
    static bool saved_first = false;
    if (!saved_first) {
        cv::imwrite("seam_visualized.jpg", seam_img);
        printf("[+] Saved visual seam snapshot to seam_visualized.jpg\n");
        saved_first = true;
    }
}

// Removes designated seam from image buffer
unsigned char* remove_vertical_seam(unsigned char* buf, int h, int w, int* seam) {
    unsigned char* out = (unsigned char*)malloc(h * (w - 1) * 3 * sizeof(unsigned char));
    for (int y = 0; y < h; y++) {
        int ox = 0;
        for (int x = 0; x < w; x++) {
            if (x == seam[y]) continue;
            for (int c = 0; c < 3; c++) {
                out[idx3(y, ox, c, w - 1)] = buf[idx3(y, x, c, w)];
            }
            ox++;
        }
    }
    return out;
}

// Transposes raw buffer for height reduction
unsigned char* transpose_buf(unsigned char* buf, int h, int w) {
    unsigned char* out = (unsigned char*)malloc(w * h * 3 * sizeof(unsigned char));
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            for (int c = 0; c < 3; c++) {
                out[idx3(x, y, c, h)] = buf[idx3(y, x, c, w)];
            }
        }
    }
    return out;
}

// Sequentially reduces width while keeping memory pointers valid
unsigned char* reduce_width(unsigned char* buf, int h, int* w, int target_w) {
    unsigned char* cur = buf;
    while (*w > target_w) {
        double* energy = compute_energy(cur, h, *w);
        int* seam = find_vertical_seam(energy, h, *w);

        // Render the red seam line
        visualize_seam(cur, h, *w, seam, "Seam Carving Process");

        unsigned char* next = remove_vertical_seam(cur, h, *w, seam);

        free(cur);
        free(energy);
        free(seam);

        cur = next;
        (*w)--;
    }
    return cur;
}

// Reduces height by transposing, running width reduction, and transposing back
unsigned char* reduce_height(unsigned char* buf, int* h, int w, int target_h) {
    unsigned char* t = transpose_buf(buf, *h, w);
    free(buf);

    int tw = *h;
    int th = w;
    t = reduce_width(t, th, &tw, target_h);

    unsigned char* back = transpose_buf(t, th, tw);
    free(t);

    *h = tw;
    return back;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <input_image> [output_image]\n", argv[0]);
        return 1;
    }
    const char* input_path = argv[1];
    const char* output_path = (argc >= 3) ? argv[2] : "output.jpg";

    cv::Mat img = cv::imread(input_path, cv::IMREAD_COLOR);
    if (img.empty()) {
        printf("Could not read image: %s\n", input_path);
        return 1;
    }

    int h = img.rows;
    int w = img.cols;
    printf("Loaded image: %d x %d\n", h, w);

    int target_h, target_w;
    printf("Enter desired height and width: ");
    if (scanf("%d %d", &target_h, &target_w) != 2) {
        printf("Invalid input\n");
        return 1;
    }

    if (target_h <= 0 || target_w <= 0 || target_h > h || target_w > w) {
        printf("Target dimensions must be positive and not exceed original (%d x %d)\n", h, w);
        return 1;
    }

    // Create the display window
    cv::namedWindow("Seam Carving Process", cv::WINDOW_AUTOSIZE);

    unsigned char* buf = mat_to_buf(img, h, w);

    buf = reduce_width(buf, h, &w, target_w);
    buf = reduce_height(buf, &h, w, target_h);

    cv::Mat out_img = buf_to_mat(buf, h, w);
    free(buf);

    cv::imwrite(output_path, out_img);
    printf("[+] Saved final resized image (%d x %d) to %s\n", h, w, output_path);

    // Keep the final frame visible until a key press
    printf("Press any key on the image window to close...\n");
    cv::imshow("Seam Carving Process", out_img);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}