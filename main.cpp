#include "RenderTools.h"

#include <iostream>

int main() {
    // New scene
    std::string path_to_scene_description = "../data/cornel_box0.shp";
    Scene scene;

    if (0) {
        if (int exit_code = scene.loadCornellBox(path_to_scene_description); exit_code != 0) {
            std::cout << "Scene: failed to load scene description: " << exit_code << std::endl;
        }
    } else {

        std::map<std::string, double> bright_coefs;

        bright_coefs.insert(std::make_pair("r", 1));
        bright_coefs.insert(std::make_pair("g", 1));
        bright_coefs.insert(std::make_pair("b", 1));

        std::map<std::string, double> rgbGrass;
        rgbGrass.insert(std::make_pair("r", 0.1));
        rgbGrass.insert(std::make_pair("g", 0.4));
        rgbGrass.insert(std::make_pair("b", 0.1));
        scene.setNewMaterial({rgbGrass, bright_coefs});

        std::map<std::string, double> rgbSky;
        rgbSky.insert(std::make_pair("r", 0.05));
        rgbSky.insert(std::make_pair("g", 0.9));
        rgbSky.insert(std::make_pair("b", 0.99));
        scene.setNewMaterial({rgbSky, bright_coefs});

        std::map<std::string, double> redBox;
        redBox.insert(std::make_pair("r", 0.8));
        redBox.insert(std::make_pair("g", 0.2));
        redBox.insert(std::make_pair("b", 0.3));
        scene.setNewMaterial({redBox, bright_coefs});

        std::map<std::string, double> whiteBox;
        whiteBox.insert(std::make_pair("r", 0.3));
        whiteBox.insert(std::make_pair("g", 0.3));
        whiteBox.insert(std::make_pair("b", 0.3));
        scene.setNewMaterial({whiteBox, bright_coefs});

        std::map<std::string, double> mirror;
        mirror.insert(std::make_pair("r", 1.0));
        mirror.insert(std::make_pair("g", 1.0));
        mirror.insert(std::make_pair("b", 1.0));
        Material materialOfMirror = Material{mirror, bright_coefs};
        materialOfMirror.BRDF = 0.7;
        scene.setNewMaterial(materialOfMirror);

        std::map<std::string, double> mirrorBox;
        mirrorBox.insert(std::make_pair("r", 0.90588));
        mirrorBox.insert(std::make_pair("g", 0.8196));
        mirrorBox.insert(std::make_pair("b", 0.078));
        Material materialOfMirrorBox = Material{mirrorBox, bright_coefs};
        materialOfMirrorBox.BRDF = 0.3;
        scene.setNewMaterial(materialOfMirrorBox);

        cv::Vec3d v0G{-500, 0, 50};
        cv::Vec3d v1G{1000, 0, 50};
        cv::Vec3d v2G{1000, 0, -2000};
        cv::Vec3d v3G{-500, 0, -2000};
        scene.addPlane(v0G, v1G, v2G, v3G, 0);

        v0G = {10000, -5000, -2000};
        v1G = {-2000, -5000, -2000};
        v2G = {-2000, 5000, -2000};
        v3G = {10000, 5000, -2000};
        scene.addPlane(v0G, v1G, v2G, v3G, 1);

        v0G = {100, 0, -300};
        v1G = {250, 0, -400};
        v2G = {250, 150, -400};
        v3G = {100, 150, -300};
        scene.addCub(v0G, v1G, v2G, v3G, 2, 100);

        v0G = {600, 0, -150};
        v1G = {480, 0, -800};
        v2G = {480, 100, -800};
        v3G = {600, 100, -150};
        scene.addCub(v0G, v1G, v2G, v3G, 3, 100);

        v0G = {750, 0, -900};
        v1G = {300, 0, -1100};
        v2G = {200, 500, -1000};
        v3G = {650, 500, -800};
        scene.addPlane(v0G, v1G, v2G, v3G, 4);

        v0G = {0,  0,  -75};
        v1G = {150, 0,  -50};
        v2G = {150, 200,-50};
        v3G = {0,  200,-75};
        scene.addCub(v0G, v1G, v2G, v3G, 5, 150);

        std::string path_to_teapot = "../data/teapot.txt";

        /*if (int exit_code = scene.loadTeapot(path_to_teapot); exit_code != 0) {
            std::cout << "Scene: failed to load teapot: " << exit_code << std::endl;
        }*/

        // Lights
        int total_intensity = 1.816936e+07; // W/sr
        double Isim_r = 900;
        double Isim_g = 600;
        double Isim_b = 600;
        double Itotal = Isim_r + Isim_g + Isim_b;

        std::map<std::string, double> spec_intensity;
        spec_intensity.insert(
                std::make_pair("r", total_intensity * (Isim_r / Itotal)));
        spec_intensity.insert(
                std::make_pair("g", total_intensity * (Isim_g / Itotal)));
        spec_intensity.insert(
                std::make_pair("b", total_intensity * (Isim_b / Itotal)));

        Light a = Light(cv::Vec3d(600, 600, -500), total_intensity,
                        spec_intensity);
        scene.setNewLight(a);
    }
    // New camera
    int width =  1280;
    int height = 720;
    double fov = M_PI / 3.f;
    cv::Vec3d origin({250, 275, 500});
    Camera camera(width, height, fov, origin);
    scene.setNewCamera(camera);

    bool antialiasing = 1;
    scene.render(antialiasing);
    return 0;
}