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

//==============================================================================
//================================ Ray =========================================
//==============================================================================
Ray::Ray(const cv::Vec3d &origin, const cv::Vec3d &direction) : origin(origin),
                                                                direction(direction) {
    cv::Vec3d L{1,1,1};
    this->L = L;
}

Ray::Ray(const cv::Vec3d &new_origin, const cv::Vec3d &new_direction,
         cv::Vec3d L) :
        origin(new_origin), direction(new_direction),
        L(std::move(L)) {
}

Ray::Ray(cv::Vec3d L) : L(std::move(L)) {
}

Ray Ray::MakeBlackRay() {
    cv::Vec3d L {0,0,0};
    return Ray(L);
}

//==============================================================================
//================================ Material ====================================
//==============================================================================
Material::Material(cv::Vec3d rgb_Kd_color, cv::Vec3d bright_coefficient) :
        rgb_Kd_color(std::move(rgb_Kd_color)), bright_coefficient(std::move(bright_coefficient)) {
}

//==============================================================================
//================================ Triangle ====================================
//==============================================================================
Triangle::Triangle(const cv::Vec3d &v0, const cv::Vec3d &v1,
                   const cv::Vec3d &v2) : v0(v0), v1(v1), v2(v2) {
}

Triangle::Triangle(const cv::Vec3d &v0, const cv::Vec3d &v1, const cv::Vec3d &v2,
                   const Material *material) : v0(v0), v1(v1), v2(v2), material(material) {

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
Light::Light(const cv::Vec3d &new_position) : position(
        new_position) {
}

Light::Light(const cv::Vec3d &new_position, cv::Vec3d new_spec_intensity) : position(new_position),
             rgb_intensity(std::move(new_spec_intensity)) {
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

cv::Vec3d Camera::getPosition() const {
    return origin;
}

//==============================================================================
//================================ Scene =======================================
//==============================================================================
Scene::Scene() {

}

void Scene::setNewCamera(const Camera &camera) {
    cameras.emplace_back(camera);
}

void Scene::setNewMaterial(const Material& material) {
    materials.emplace_back(material);
}

void Scene::setNewLight(const Light &light) {
    lights.emplace_back(light);
}

void Scene::setNewTriangle(const cv::Vec3d &v0, const cv::Vec3d &v1, const cv::Vec3d &v2, const int &id) {
    triangles.emplace_back(Triangle(v0, v1, v2, &materials[id]));
}

void Scene::addPlane(const cv::Vec3d &v0, const cv::Vec3d &v1, const cv::Vec3d &v2,
                     const cv::Vec3d &v3, const int &id) {
    triangles.emplace_back(Triangle(v0, v1, v2, &materials[id]));
    triangles.emplace_back(Triangle(v2, v3, v0, &materials[id]));
}

void Scene::addCube(const cv::Vec3d &v0, const cv::Vec3d &v1, const cv::Vec3d &v2,
                    const cv::Vec3d &v3, const int &id, const double &depth) {
    cv::Vec3d normal = get_normalized((v0 - v1).cross(v2 - v1));
    cv::Vec3d v00 = v0 + normal * depth;
    cv::Vec3d v10 = v1 + normal * depth;
    cv::Vec3d v20 = v2 + normal * depth;
    cv::Vec3d v30 = v3 + normal * depth;
    addPlane(v0, v1, v2, v3, id);
    addPlane(v0, v1, v10, v00, id);
    addPlane(v1, v2, v20, v10, id);
    addPlane(v2, v3, v30, v20, id);
    addPlane(v3, v0, v00, v30, id);
    addPlane(v00, v10, v20, v30, id);
}

void Scene::setAntialiasing(const bool & antialiasing) {
    this->antialiasing = antialiasing;
}
void Scene::setLightInShadows(const bool & lightInShadows) {
    this->lightInShadows = lightInShadows;
}

bool Scene::intersect(const Ray &ray, cv::Vec3d &positionOfHit, cv::Vec3d &N, Material &material) {

    double min_distance_to_triangle = std::numeric_limits<double>::max();

    for (auto &triangle : triangles) {
        double distance_to_triangle = -1.0;
        if (triangle.intersect(ray, distance_to_triangle) &&
            distance_to_triangle < min_distance_to_triangle) {
            min_distance_to_triangle = distance_to_triangle;
            positionOfHit = ray.origin + ray.direction * distance_to_triangle;
            N = triangle.getNormalByObserver(ray.origin - positionOfHit);
            material = triangle.giveMaterial();
        }
    }

    return min_distance_to_triangle < std::numeric_limits<double>::max();
}

Ray Scene::fireRay(Ray &ray) {
    cv::Vec3d hitRayIntersection, N;
    Material material;

    if (!intersect(ray, hitRayIntersection, N, material)) {
        return ray.MakeBlackRay();  //ray intersect no one triangle
    }

    Ray rasultRay(cv::Vec3d{0,0,0});

    for (int i = 0; i < lights.size(); i++) {

        cv::Vec3d light_dir = get_normalized(lights[i].givePosition() - hitRayIntersection);
        double dist = get_length(lights[i].givePosition() - hitRayIntersection);
        double cos_theta = light_dir.dot(N);

        if ((material.giveBRDF() != 0) && ((ray.L[0] + ray.L[1] + ray.L[2]) > 0.0005)) {   //reflection
            cv::Vec3d reflactionL = material.giveBRDF() * material.giveRGBCOlod().mul(ray.L);
            Ray reflectionRay = Ray(hitRayIntersection, get_normalized(ray.direction + 2 * N), reflactionL);
            reflectionRay = fireRay(reflectionRay);
            rasultRay.L = reflectionRay.L;
        }

        if (cos_theta <= 0 && lightInShadows) {   //triangle unlit
                cv::Vec3d E =-0.008 * lights[i].giveRGBIntensity() * cos_theta / pow(dist, 2);
                rasultRay.L += (1- material.giveBRDF())*E.mul(material.giveRGBCOlod().mul(ray.L));
        }
        else {
            cv::Vec3d shadow_origin = hitRayIntersection + N * 1e-3;
            Ray shadow_ray = Ray(shadow_origin, light_dir);
            cv::Vec3d shadow_hit, shadow_N;
            Material shadow_material;

            if (intersect(shadow_ray, shadow_hit, shadow_N, shadow_material) &&
                get_length(shadow_hit - shadow_origin) < dist && lightInShadows) {        //triangle in shadow
                    cv::Vec3d E = 0.008 * lights[i].giveRGBIntensity() * cos_theta / pow(dist, 2);
                    rasultRay.L += (1- material.giveBRDF())*E.mul(material.giveRGBCOlod().mul(ray.L));
            }else {
                cv::Vec3d E = lights[i].giveRGBIntensity() * cos_theta / (dist * dist);
                rasultRay.L += (1 - material.giveBRDF()) * E.mul(material.giveRGBCOlod().mul(ray.L));
            }
        }
    }
    rasultRay.L /= M_PI;
    return rasultRay;
}

int Scene::loadCornellBox(const std::string &path_to_file) {
    // Materials
    cv::Vec3d bright_coefs(1,1,1);

    cv::Vec3d rgb_Kd_color_brs_0 {0.735922, 0.7363956, 0.7373938};
    materials.emplace_back(Material(rgb_Kd_color_brs_0, bright_coefs));

    cv::Vec3d rgb_Kd_color_brs_1 {0.09629525, 0.4120723, 0.08873872};
    materials.emplace_back(Material(rgb_Kd_color_brs_1, bright_coefs));

    cv::Vec3d rgb_Kd_color_brs_2 {0.4407372, 0.04373143, 0.04556723};
    materials.emplace_back(Material(rgb_Kd_color_brs_2, bright_coefs));

    cv::Vec3d rgb_Kd_color_brs_3 {0.735922, 0.7363956, 0.7373938};
    materials.emplace_back(Material(rgb_Kd_color_brs_3, bright_coefs));

    // Lights
    int total_intensity = 24'766'690*42.4115; // W/sr
    double Isim_r =  1.000;
    double Isim_g =  0.569;
    double Isim_b =  0.135;
    double Itotal = Isim_r + Isim_g + Isim_b;

    cv::Vec3d rgb_intensity {total_intensity * (Isim_r / Itotal),
                             total_intensity * (Isim_g / Itotal),
                             total_intensity * (Isim_b / Itotal)};

    Light a = Light(cv::Vec3d(278, 548.7, -279.5), rgb_intensity);
    lights.push_back(a);

    // Triangles
    int exit_code = 0;

    std::string s;
    std::ifstream file(path_to_file);
    std::vector<cv::Vec3d> points;

    while (getline(file, s)) {

        if (s.find("Number of vertices") != std::string::npos) {
            std::istringstream iss(s);
            std::string oneString;
            iss >> oneString;
            iss >> oneString;
            points.clear();
            points.resize(stod(oneString));
            for(int i = 0; i < stod(oneString); i++){
                getline(file, s);
                std::istringstream oss(s);
                cv::Vec3d pointPosition;
                int j = 0;
                for (std::string temp; oss >> temp;){
                    pointPosition[j++] = stod(temp);
                }
                points[i] = pointPosition;
            }
        }

        if (s.find("Number of triangles") != std::string::npos) {
            std::istringstream iss(s);
            std::string oneString;
            iss >> oneString;
            iss >> oneString;
            for(int i = 0; i < stod(oneString); i++){
                getline(file, s);
                std::istringstream oss(s);
                cv::Vec3i numberOfVertex;
                int j = 0;
                for (std::string temp; oss >> temp;){
                    numberOfVertex[j++] = stod(temp);
                }
                if(stod(oneString) == 32)
                    setNewTriangle( points[numberOfVertex[0]], points[numberOfVertex[1]], points[numberOfVertex[2]], 0);
                if(stod(oneString) == 2)
                    setNewTriangle( points[numberOfVertex[0]], points[numberOfVertex[1]], points[numberOfVertex[2]], 1);
                if(stod(oneString) == 6)
                    setNewTriangle( points[numberOfVertex[0]], points[numberOfVertex[1]], points[numberOfVertex[2]], 2);
                if(stod(oneString) == 12)
                    setNewTriangle( points[numberOfVertex[0]], points[numberOfVertex[1]], points[numberOfVertex[2]], 3);
            }
        }

    }

    file.close();

    return exit_code;
}

int Scene::loadTeapot(const std::string &path_to_file) {

    // Triangles
    int exit_code = 0;

    std::string s;
    std::ifstream file(path_to_file);

    std::vector<cv::Vec3d> allPoint;
    int counter = 0;
    cv::Vec3d translation = {400,0,-300};
    while (getline(file, s)) {

        std::istringstream iss(s);
        std::string vOrF;
        iss >> vOrF;
        if (vOrF == "v"){
            std::vector<double> cords;
            for (std::string s; iss >> s;)
                cords.push_back(stod(s) * 85);
            cv::Vec3d new_point = cv::Vec3d(cords[0], cords[1],
                                            cords[2]); // ???????? cv::Vec3d(cords[0], cords[1], cords[2])
            allPoint.push_back(new_point + translation);
        }

        if (vOrF == "f") {
            std::vector<int> number_of_triangles;
            for (std::string s; iss >> s;)
                number_of_triangles.push_back(stoi(s));
            cv::Vec3d v0 = allPoint[number_of_triangles[0]-1];
            cv::Vec3d v1 = allPoint[number_of_triangles[1]-1];
            cv::Vec3d v2 = allPoint[number_of_triangles[2]-1];
            this->setNewTriangle(v0, v1, v2, 3);
        }
    }
    file.close();

    return exit_code;
}


void Scene::render() {
    std::cout << triangles.size() << std::endl;
    for (auto &camera : cameras) {
        int width = camera.getWidth();
        int height = camera.getHeight();
        double fov = camera.getFov();
        std::vector<Ray> framebuffer(width * height);
        if (antialiasing) {
            width = width*2+1;
            height = height*2+1;
            std::vector<Ray> antialiasingFrameBuffer(width * height);

            for (long long i = 0; i < height; i++) {
                for (long long j = 0; j < width; j++) {
                    double x = -(2 * (j + 0.5) / (double) width - 1) * tan(fov / 2.) * width /
                               (double) height;
                    double y = -(2 * (i + 0.5) / (double) height - 1) * tan(fov / 2.);
                    cv::Vec3d direction = get_normalized(cv::Vec3d(x, y, -1));

                    Ray ray(camera.getPosition(), direction);
                    antialiasingFrameBuffer[j + i * width] = fireRay(ray);
                }
            }

            width = (width-1)/2;
            height = (height-1)/2;

            for (long long i = 0; i < height; i++) {
                for (long long j = 0; j < width; j++) {
                    int position_X = i * 2 + 1;
                    int position_Y = j * 2 + 1;
                    Ray result = antialiasingFrameBuffer[0].MakeBlackRay();
                    result.L = (   antialiasingFrameBuffer[position_Y - 1 + (position_X - 1) * (width * 2 + 1)].L +
                                   antialiasingFrameBuffer[position_Y +     (position_X - 1) * (width * 2 + 1)].L +
                                   antialiasingFrameBuffer[position_Y + 1 + (position_X - 1) * (width * 2 + 1)].L +
                                   antialiasingFrameBuffer[position_Y - 1 + (position_X) *     (width * 2 + 1)].L +
                                   antialiasingFrameBuffer[position_Y +     (position_X) *     (width * 2 + 1)].L +
                                   antialiasingFrameBuffer[position_Y + 1 + (position_X) *     (width * 2 + 1)].L +
                                   antialiasingFrameBuffer[position_Y - 1 + (position_X + 1) * (width * 2 + 1)].L +
                                   antialiasingFrameBuffer[position_Y +     (position_X + 1) * (width * 2 + 1)].L +
                                   antialiasingFrameBuffer[position_Y + 1 + (position_X + 1) * (width * 2 + 1)].L) / 9;

                    framebuffer[j + i * width] = result;
                }
            }


        } else {
            for (long long i = 0; i < height; i++) {
                for (long long j = 0; j < width; j++) {

                    double x = -(2 * (j + 0.5) / (double) width - 1) * tan(fov / 2.) * width /
                               (double) height;
                    double y = -(2 * (i + 0.5) / (double) height - 1) * tan(fov / 2.);
                    cv::Vec3d direction = get_normalized(cv::Vec3d(x, y, -1));
                    Ray ray(camera.getPosition(), direction);
                    framebuffer[j + i * width] = fireRay(ray);
                }
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
                        fout << framebuffer[j + i * width].L[k];
                    } else {
                        fout << " "
                             << framebuffer[j + i * width].L[k];
                    }
                }
                fout << std::endl;
            }
            fout << "\n";
        }

        fout.close();
    }
}