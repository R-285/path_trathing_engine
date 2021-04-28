#ifndef RENDER_TOOLS_H
#define RENDER_TOOLS_H
#define _USE_MATH_DEFINES

#include <opencv2/core/matx.hpp>

#include <vector>
#include <cmath>
#include <map>

//==============================================================================
//================================ Free functions ==============================
//==============================================================================
cv::Vec3d get_normalized(const cv::Vec3d &vec);
double get_length(const cv::Vec3d &vec);

//==============================================================================
//================================ Ray =========================================
//==============================================================================
class Ray {
public:
    Ray() = default;
    Ray(cv::Vec3d color);
    Ray(const cv::Vec3d &origin, const cv::Vec3d &direction);
    Ray(const cv::Vec3d &new_origin, const cv::Vec3d &new_direction, cv::Vec3d color);
    Ray MakeBlackRay();
public:
    cv::Vec3d origin;
    cv::Vec3d direction;
    cv::Vec3d L;
};

//==============================================================================
//================================ Material ====================================
//==============================================================================
class Material {
public:
    Material() = default;
    Material(cv::Vec3d rgb_Kd_color,  cv::Vec3d bright_coefficient);
    void setBRDF(double newBRDF){
        BRDF = std::min(newBRDF, 0.99999);
        if (BRDF < 0) BRDF = 0;
    }
    double giveBRDF(){ return BRDF;};
    cv::Vec3d giveRGBCOlod(){return rgb_Kd_color;}

private:
    double BRDF = 0.0;
    cv::Vec3d rgb_Kd_color;
    cv::Vec3d bright_coefficient;
};

//==============================================================================
//================================ Triangle ====================================
//==============================================================================

class Triangle {
public:
    Triangle() = default;

    Triangle(const cv::Vec3d &v0, const cv::Vec3d &v1, const cv::Vec3d &v2);
    Triangle(const cv::Vec3d &v0, const cv::Vec3d &v1, const cv::Vec3d &v2,
             const Material *material);
    cv::Vec3d getNormalByObserver(const cv::Vec3d &observer) const;
    bool intersect(const Ray &ray, double &t) const;
    Material giveMaterial() const{
        return *material;
    }

private:
    cv::Vec3d v0, v1, v2;
    const Material *material;
};

//==============================================================================
//================================ Light =======================================
//==============================================================================
struct Light {
public:
    Light() = default;
    Light(const cv::Vec3d &new_position);
    Light(const cv::Vec3d &new_position,
          cv::Vec3d new_rgb_intensity);
    cv::Vec3d givePosition(){
        return position;
    }
    cv::Vec3d giveRGBIntensity(){
        return rgb_intensity;
    }
private:
    cv::Vec3d position;
    cv::Vec3d rgb_intensity;
};

//==============================================================================
//================================ Camera ======================================
//==============================================================================
class Camera {
public:
    Camera(int width, int height, double fov, const cv::Vec3d &origin);

    void getPicture();

    int getWidth() const;
    int getHeight() const;
    double getFov() const;
    cv::Vec3d getPosition() const;

private:
    const int width = -1;
    const int height = -1;
    const double fov = -1.0;

    const cv::Vec3d origin = {-1.0, -1.0, -1.0};
};

//==============================================================================
//================================ Scene =======================================
//==============================================================================
class Scene {
public:
    Scene();
    void render();
    Ray fireRay(Ray &ray);
    int loadCornellBox(const std::string &path_to_file);
    int loadTeapot(const std::string &path_to_file);

    void addPlane(const cv::Vec3d& v0,const cv::Vec3d& v1,const cv::Vec3d& v2,
                  const cv::Vec3d& v4, const int & id);
    void addCube(const cv::Vec3d& v0, const cv::Vec3d& v1, const cv::Vec3d& v2,
                 const cv::Vec3d& v3, const int & id, const double& depth);
    void setNewTriangle(const cv::Vec3d& v0,const cv::Vec3d& v1,const cv::Vec3d& v2,
                        const int & id);

    void setNewCamera(const Camera &camera);
    void setNewMaterial(const Material& material);
    void setNewLight(const Light & light);

    void setAntialiasing(const bool & antialiasing);
    void setLightInShadows(const bool & lightInShadows);
private:
    bool intersect(const Ray &ray, cv::Vec3d &positionOfHit, cv::Vec3d &N, Material &material);

private:
    bool antialiasing = false;
    bool lightInShadows = false;
    std::vector<Triangle> triangles;
    std::vector<Light> lights;
    std::vector<Material> materials;
    std::vector<Camera> cameras;
};

#endif //RENDER_TOOLS_H