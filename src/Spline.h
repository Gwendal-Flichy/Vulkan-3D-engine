#include <iostream>
#include <vector>
#include <cmath>

struct Vec3 {
    double x, y, z;

    Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& b) const {
        return Vec3(x + b.x, y + b.y, z + b.z);
    }

    Vec3 operator-(const Vec3& b) const {
        return Vec3(x - b.x, y - b.y, z - b.z);
    }

    Vec3 operator*(double s) const {
        return Vec3(x * s, y * s, z * s);
    }
};

double distance(const Vec3& a, const Vec3& b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double dz = b.z - a.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

Vec3 catmull_rom(const Vec3& P0, const Vec3& P1, const Vec3& P2, const Vec3& P3, double t, double alpha)
{

    double t0 = 0.0;
    double t1 = t0 + std::pow(distance(P0, P1), alpha);
    double t2 = t1 + std::pow(distance(P1, P2), alpha);
    double t3 = t2 + std::pow(distance(P2, P3), alpha);

    double tr = t1 + t * (t2 - t1);

    Vec3 A1 = P0 * ((t1 - tr) / (t1 - t0)) + P1 * ((tr - t0) / (t1 - t0));
    Vec3 A2 = P1 * ((t2 - tr) / (t2 - t1)) + P2 * ((tr - t1) / (t2 - t1));
    Vec3 A3 = P2 * ((t3 - tr) / (t3 - t2)) + P3 * ((tr - t2) / (t3 - t2));

    Vec3 B1 = A1 * ((t2 - tr) / (t2 - t0)) + A2 * ((tr - t0) / (t2 - t0));
    Vec3 B2 = A2 * ((t3 - tr) / (t3 - t1)) + A3 * ((tr - t1) / (t3 - t1));

    Vec3 C = B1 * ((t2 - tr) / (t2 - t1)) + B2 * ((tr - t1) / (t2 - t1));

    return C;
}

std::vector<Vec3> inti_rail(const std::vector<Vec3>& points) {
    std::vector<Vec3> rail;

    int n = points.size();

    Vec3 faux_debut = points[0] + (points[0] - points[1]);
    rail.push_back(faux_debut);

    for (const Vec3& p : points) {
        rail.push_back(p);
    }

    Vec3 faux_fin = points[n - 1] + (points[n - 1] - points[n - 2]);
    rail.push_back(faux_fin);

    return rail;
}

//int main()
//{
//    std::vector<Vec3> points = {
//        Vec3(0.0,  0.0,  0.0),
//        Vec3(1.0,  2.0,  0.5),
//        Vec3(3.0,  3.0,  1.0),
//        Vec3(5.0,  1.0,  2.0),
//        Vec3(6.0,  0.0,  1.5),
//        Vec3(7.0,  2.0,  0.0),
//    };
//
//    std::vector<Vec3> rail = inti_rail(points);
//
//    double alpha = 0.5;
//    int pas = 100;
//
//    int taille = rail.size();
//
//    for (int i = 1; i <= taille - 3; i++) {
//
//        Vec3 P0 = rail[i - 1];
//        Vec3 P1 = rail[i];
//        Vec3 P2 = rail[i + 1];
//        Vec3 P3 = rail[i + 2];
//
//        for (int j = 0; j < pas; j++) {
//
//            double t = (double)j / (double)pas;
//
//            Vec3 pos = catmull_rom(P0, P1, P2, P3, t, alpha);
//
//            std::cout << "x: " << pos.x
//                << "  y: " << pos.y
//                << "  z: " << pos.z
//                << "\n";
//        }
//    }
//    std::cout << "x: " << points.back().x
//        << "  y: " << points.back().y
//        << "  z: " << points.back().z
//        << "\n";
//
//    return 0;
//}