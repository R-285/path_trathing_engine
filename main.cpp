#include "RenderTools.h"

#include <iostream>

int main() {
  // New scene
  std::string path_to_scene_description = "../data/cornel_box0.shp";
  Scene scene;
  if (int exit_code = scene.load(path_to_scene_description); exit_code != 0) {
    std::cout << "Scene: failed to load scene description: " << exit_code
              << std::endl;
  }

  // New camera
  int width = 1024;
  int height = 768;
  double fov = M_PI / 3.f;
  cv::Vec3d origin({250, 275, 500});
  Camera camera(width, height, fov, origin);
  scene.setNewCamera(camera);

  scene.render();
  return 0;
}