#include "RenderTools.h"

#include <fstream>
#include <iostream>
#include <sstream>

//==============================================================================
//================================ Free functions ==============================
//==============================================================================
double get_length(const cv::Vec3d &vec) {
  return std::sqrt(
      std::pow(vec[0], 2) + std::pow(vec[1], 2) + std::pow(vec[2], 2));
}

cv::Vec3d get_normalized(const cv::Vec3d &vec) {
  double length = get_length(vec);

  if (length == 0) {
    return vec;
  }
  return vec / length;
}

Ray fill_wavelength(Ray ray, std::vector<Light> lights) {
  std::map<std::string, double> bright_coefs;
  std::map<std::string, double> L;

  for (int i = 0; i < lights.size(); i++) {
    for (auto &item : lights[i].spec_intensity) {
      bright_coefs.insert(std::make_pair(item.first, 1));
      L.insert(std::make_pair(item.first, 0));
    }
  }

  ray.bright_coefficient = bright_coefs;
  ray.L = L;

  return ray;
}

//==============================================================================
//================================ Ray =========================================
//==============================================================================
Ray::Ray(const cv::Vec3d &origin, const cv::Vec3d &direction) : origin(origin),
                                                                direction(
                                                                    direction) {
}

Ray::Ray(const cv::Vec3d &new_origin, const cv::Vec3d &new_direction,
         std::map<std::string, double> bright_coefs,
         std::map<std::string, double> L) :
    origin(origin), direction(direction),
    bright_coefficient(std::move(bright_coefs)), L(std::move(L)) {
}

//==============================================================================
//================================ Material ====================================
//==============================================================================
Material::Material(std::map<std::string, double> rgb_Kd_color) : rgb_Kd_color(
    std::move(rgb_Kd_color)) {
}

//==============================================================================
//================================ Triangle ====================================
//==============================================================================
Triangle::Triangle(const cv::Vec3d &v0, const cv::Vec3d &v1,
                   const cv::Vec3d &v2) : v0(v0), v1(v1), v2(v2) {
}

Triangle::Triangle(const cv::Vec3d &v0, const cv::Vec3d &v1,
                   const cv::Vec3d &v2,
                   const int &object_id) : v0(v0), v1(v1), v2(v2),
                                           object_id(object_id) {
}

Triangle::Triangle(const cv::Vec3d &v0, const cv::Vec3d &v1,
                   const cv::Vec3d &v2,
                   const int &object_id,
                   const Material *material) :
    v0(v0), v1(v1), v2(v2), object_id(object_id), material(material) {
}

cv::Vec3d Triangle::getNormal() const {
  cv::Vec3d normal = (v1 - v0).cross(v2 - v0);
  normal = get_normalized(normal);
  return normal;
}

cv::Vec3d Triangle::getNormalByObserver(const cv::Vec3d &observer) const {
  cv::Vec3d normal = (v1 - v0).cross(v2 - v0);

  normal = get_normalized(normal);

  if (get_normalized(observer).dot(normal) < 0) {
    normal = (v2 - v0).cross(v1 - v0);
    normal = get_normalized(normal);
  }

  return normal;
}

bool Triangle::intersect(const Ray &ray, double &t) const {
  cv::Vec3d e1 = v1 - v0;
  cv::Vec3d e2 = v2 - v0;
  cv::Vec3d pvec = ray.direction.cross(e2);
  double det = e1.dot(pvec);

  if (det < 1e-8 && det > -1e-8) {
    return false;
  }

  double inv_det = 1 / det;
  cv::Vec3d tvec = ray.origin - v0;
  double u = tvec.dot(pvec) * inv_det;
  if (u < 0 || u > 1) {
    return false;
  }

  cv::Vec3d qvec = tvec.cross(e1);
  double v = ray.direction.dot(qvec) * inv_det;
  if (v < 0 || v + u > 1) {
    return false;
  }

  t = e2.dot(qvec) * inv_det;
  return t > 1e-8;
}

//==============================================================================
//================================ Light =======================================
//==============================================================================
Light::Light(const cv::Vec3d &new_position, double new_total_flux) : position(
    new_position), total_intensity(new_total_flux) {
}

Light::Light(const cv::Vec3d &new_position, double new_total_flux,
             std::map<std::string, double> new_spec_intensity) : position(
    new_position), total_intensity(new_total_flux), spec_intensity(std::move(
    new_spec_intensity)) {
}

//==============================================================================
//================================ Camera ======================================
//==============================================================================
Camera::Camera(int width, int height, double fov, const cv::Vec3d &origin)
    : width(width), height(height), fov(fov), origin(origin) {
}

int Camera::getWidth() const {
  return width;
}

int Camera::getHeight() const {
  return height;
}

double Camera::getFov() const {
  return fov;
}

cv::Vec3d Camera::getPosition() const{
    return origin;
}

//FIXME: Rename this method
void Camera::getPicture() {
}

//==============================================================================
//================================ Scene =======================================
//==============================================================================
Scene::Scene() {
  // Materials
  //Хардкод, так как в файле нет спектральных свойств поверхностей
  std::map<std::string, double> rgb_Kd_color_brs_0;
  rgb_Kd_color_brs_0.insert(std::make_pair("r", 0.238));
  rgb_Kd_color_brs_0.insert(std::make_pair("g", 0.199));
  rgb_Kd_color_brs_0.insert(std::make_pair("b", 0.158));
  materials.emplace_back(Material(rgb_Kd_color_brs_0));

  std::map<std::string, double> rgb_Kd_color_brs_1;
  rgb_Kd_color_brs_1.insert(std::make_pair("r", 0.096));
  rgb_Kd_color_brs_1.insert(std::make_pair("g", 0.412));
  rgb_Kd_color_brs_1.insert(std::make_pair("b", 0.089));
  materials.emplace_back(Material(rgb_Kd_color_brs_1));

  std::map<std::string, double> rgb_Kd_color_brs_2;
  rgb_Kd_color_brs_2.insert(std::make_pair("r", 0.441));
  rgb_Kd_color_brs_2.insert(std::make_pair("g", 0.044));
  rgb_Kd_color_brs_2.insert(std::make_pair("b", 0.046));
  materials.emplace_back(Material(rgb_Kd_color_brs_2));

  std::map<std::string, double> rgb_Kd_color_brs_3;
  rgb_Kd_color_brs_3.insert(std::make_pair("r", 0.104));
  rgb_Kd_color_brs_3.insert(std::make_pair("g", 0.38));
  rgb_Kd_color_brs_3.insert(std::make_pair("b", 0.44));
  materials.emplace_back(Material(rgb_Kd_color_brs_3));

  std::map<std::string, double> rgb_Kd_color_brs_4;
  rgb_Kd_color_brs_4.insert(std::make_pair("r", 0.104));
  rgb_Kd_color_brs_4.insert(std::make_pair("g", 0.38));
  rgb_Kd_color_brs_4.insert(std::make_pair("b", 0.44));
  materials.emplace_back(Material(rgb_Kd_color_brs_4));

  // Lights
  //Хардкод, так как в файле нет свойств и положения источника света
  int total_intensity = 1445872; // W/sr
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

  Light a = Light(cv::Vec3d(278, 545, -279.5), 1445872,
                  spec_intensity);
  lights.push_back(
      a); //Было Light(cv::Vec3d(278, -279.5, 548.7), 1445872, spec_intensity)
  //278, 548.7, -279.5
}

void Scene::setNewCamera(const Camera &camera) {
  cameras.emplace_back(camera);
}

// FIXME: Change signature
Ray Scene::fireRay(Ray &ray) {
  cv::Vec3d hit, N;
  Material material;

  if (!intersect(ray, hit, N, material)) {
    return ray;
  }

  for (auto &item : material.rgb_Kd_color) {

    ray.bright_coefficient.find(item.first)->second *= item.second;
  }

  for (int i = 0; i < lights.size(); i++) {
    cv::Vec3d light_dir = get_normalized(lights[i].position - hit);
    double dist = get_length(lights[i].position - hit);
    double cos_teta = light_dir.dot(N);
    if (cos_teta <= 0) {
      return ray;
    }

    cv::Vec3d shadow_origin = hit + N * 1e-3;
    Ray shadow_ray = Ray(shadow_origin, light_dir);
    cv::Vec3d shadow_hit, shadow_N;
    Material shadow_material;

    if (intersect(shadow_ray, shadow_hit, shadow_N, shadow_material) &&
        get_length(shadow_hit - shadow_origin) < dist) {
      continue;
    }

    for (auto &item : ray.L) {
      double E = ((lights[i].spec_intensity.find(item.first)->second) /
                  (dist * dist)) * cos_teta;
      item.second =
          (E * ray.bright_coefficient.find(item.first)->second) / (double) M_PI;
    }
  }

  return ray;
}

// FIXME: Change signature (material)
bool Scene::intersect(const Ray &ray, cv::Vec3d &hit, cv::Vec3d &N,
                      Material &material) {

  double min_distance_to_triangle = std::numeric_limits<double>::max();

  for (auto &triangle : triangles) {
    double distance_to_triangle = -1.0;
    if (triangle.intersect(ray, distance_to_triangle) &&
        distance_to_triangle < min_distance_to_triangle) {
      min_distance_to_triangle = distance_to_triangle;
      hit = ray.origin + ray.direction * distance_to_triangle;
      N = triangle.getNormalByObserver(ray.origin - hit);
      material = *triangle.material;
    }
  }

  return min_distance_to_triangle < std::numeric_limits<double>::max();
}

int Scene::load(const std::string &path_to_file) {
  int exit_code = 0;

  std::string s;
  std::ifstream file(path_to_file);
  int state = 0; //1 - define breps brs_ найден, 2 - Number of vertices найден, Number of triangles найден
  int points_size = 0;
  while (getline(file, s)) {

    if (s.find("Number of parts") != std::string::npos) {
      state = 0;
      continue;
    }

    if (s.find("Number of triangles") != std::string::npos) {
      state = 3;
      continue;
    }

    if (state == 3) {
      std::istringstream iss(s);
      std::vector<int> number_of_triangles;
      for (std::string s; iss >> s;)
        number_of_triangles.push_back(stoi(s));

      Triangle triangle = Triangle(points[points_size + number_of_triangles[0]],
                                   points[points_size + number_of_triangles[1]],
                                   points[points_size + number_of_triangles[2]],
                                   object_id[object_id.size() - 1],
                                   &materials[object_id[object_id.size() - 1]]);
      triangles.push_back(triangle);
      continue;
    }

    if (s.find("define breps brs_") != std::string::npos) {
      std::istringstream iss(s);
      std::string token;
      while (getline(iss, token, '_')) {
      }
      object_id.push_back(stoi(token));
      state = 1;
      continue;
    }

    if (s.find("Number of vertices") != std::string::npos) {
      state = 2;
      points_size = points.size();
      continue;
    }

    if (state == 2) {
      std::istringstream iss(s);
      std::vector<double> cords;
      for (std::string s; iss >> s;)
        cords.push_back(stod(s));
      cv::Vec3d new_point = cv::Vec3d(cords[0], cords[1],
                                      cords[2]); // Было cv::Vec3d(cords[0], cords[1], cords[2])
      points.push_back(new_point);
    }
  }

  file.close();

  std::cout << triangles.size() << std::endl;

  return exit_code;
}


void Scene::render() {

  for (auto &camera : cameras) {
    int width = camera.getWidth();
    int height = camera.getHeight();
    double fov = camera.getFov();

    std::vector<Ray> framebuffer(width * height);

#pragma omp parallel for
    for (long long i = 0; i < height; i++) {
      for (long long j = 0; j < width; j++) {
        double x =
            -(2 * (j + 0.5) / (double) width - 1) * tan(fov / 2.) * width /
            (double) height;
        double y = -(2 * (i + 0.5) / (double) height - 1) * tan(fov / 2.);
        cv::Vec3d direction = get_normalized(cv::Vec3d(x, y, -1)); //Было -1

        Ray ray(camera.getPosition(),
                direction); //100, 200, 100 //250, 300, 500
        ray = fill_wavelength(ray, lights);

        framebuffer[j + i * width] = fireRay(ray);
      }
    }

    // FIXME: Change save path
    std::ofstream fout("../data/results.txt");

    std::vector<std::string> wavelengths;
    wavelengths.emplace_back("r");
    wavelengths.emplace_back("g");
    wavelengths.emplace_back("b");

    for (int k = 0; k < wavelengths.size(); k++) {
      fout << "wavelength" << " " << wavelengths[k] << std::endl;
      for (long long i = 0; i < height; i++) {
        for (long long j = 0; j < width; j++) {
          if (j == 0) {
            fout << framebuffer[j + i * width].L.find(wavelengths[k])->second;
          } else {
            fout << " "
                 << framebuffer[j + i * width].L.find(wavelengths[k])->second;
          }
        }
        fout << std::endl;
      }
      fout << "\n";
    }

    fout.close();
  }
}