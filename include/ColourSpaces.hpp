#ifndef COLOUR_SPACES_HPP
#define COLOUR_SPACES_HPP

#include <math.h>

namespace ColourSpaces{
    class XYZ;
    class RGB;
    class LinRGB;
    class LAB;

    class RGB {
    private:
        unsigned char _RGBClamp(const short int& val) const {
            return std::min(std::max((short int)0,val), (short int)255);
    }

    double _linTransform(const unsigned char& val) const {
        double linVal = (double)val/255.0;
        if (linVal <= 0.04045) {
            linVal /= 12.92;
        }
        else {
            linVal = pow((linVal + 0.055) / 1.055, 2.4);
        }
        return linVal;
    }

    public:
        unsigned char r = 0;
        unsigned char g = 0;
        unsigned char b = 0;

        RGB() = default;
        RGB(const unsigned char& R, const unsigned char& G, const unsigned char& B) : r(R), g(G), b(B) {}
        RGB(const short int& R, const short int& G, const short int& B) : r(_RGBClamp(R)), g(_RGBClamp(G)), b(_RGBClamp(B)) {}

        LinRGB toLinRGB() const;
    };

    class LinRGB {
    private:
        unsigned char _rgbTransform(double val) const {
            if (val <= 0.0031308) {
                val *= 12.92;
            }
            else {
                val = 1.055*pow(val, 1/2.4) - 0.055;
            }
            val = round(val * 255);
            return (unsigned char)val;
        }

    public:
        double r = 0;
        double g = 0;
        double b = 0;

        LinRGB() = default;
        LinRGB(const double& R, const double& G, const double& B) : r(R), g(G), b(B) {}

        RGB toRGB() const {
            return RGB(_rgbTransform(r), _rgbTransform(g), _rgbTransform(b));
        }

        XYZ toXYZ() const;
    };

    class LAB {
    private:
        double _xyzTransform(const double& val) const {
            if(val > 6.0/29){
                return pow(val, 3);
            }
            else {
                return (val - 4.0/29)*(108.0/841.0);
            }
        }

    public:
        double l = 0;
        double a = 0;
        double b = 0;

        LAB() = default;
        LAB(const double& L, const double& A, const double& B) : l(L), a(A), b(B) {}

        XYZ toXYZ() const;
    };

    class XYZ{
    private:
        double _labTransform(const double& val) const {
            if (val > 0.008856) {
                return pow(val, 1.0 / 3);
            }
            else {
                return (841.0/108.0)*val + 4.0/29;
            }
        }

    public:
        double x = 0;
        double y = 0;
        double z = 0;

        XYZ() = default;
        XYZ(const double& X, const double& Y, const double& Z) : x(X), y(Y), z(Z) {}

        XYZ operator+(const XYZ& other){
            return XYZ(
                x + other.x,
                y + other.y,
                z + other.z
            );
        }

        XYZ operator-(const XYZ& other) const {
            return XYZ(
                x - other.x,
                y - other.y,
                z - other.z
            );
        }

        XYZ operator*(const double& factor) const {
            return XYZ(
                x * factor,
                y * factor,
                z * factor
            );
        }

        XYZ operator/(const double& delimeter) const {
            return (*this)*(1/delimeter);
        }

        LinRGB toLinRGB() const {
            return LinRGB(
                (3.2406  * x - 1.5372 * y - 0.4986 * z),
                (-0.9689 * x + 1.8758 * y + 0.0415 * z),
                (0.0557  * x - 0.2040 * y + 1.0569 * z)
            );
        }

        LAB toLAB() const {
            XYZ tempC(*this);
            tempC.x /= 0.950489;
            tempC.z /= 1.08884;
            tempC.x = _labTransform(tempC.x);
            tempC.y = _labTransform(tempC.y);
            tempC.z = _labTransform(tempC.z);
            return LAB(116 * tempC.y - 16, 500 * (tempC.x - tempC.y), 200 * (tempC.y - tempC.z));
        }
    };

    LinRGB RGB::toLinRGB() const {
        return LinRGB(
            _linTransform(r),
            _linTransform(g),
            _linTransform(b)
        );
    }

    XYZ LinRGB::toXYZ() const {
        return {
            (0.4124 * r + 0.3576 * g + 0.1805 * b),
            (0.2126 * r + 0.7152 * g + 0.0722 * b),
            (0.0193 * r + 0.1192 * g + 0.9505 * b),
        };
    }

    XYZ LAB::toXYZ() const {
        XYZ xyzColour;
        xyzColour.y = (l + 16.0) / 116.0;
        xyzColour.x = xyzColour.y + a / 500.0;
        xyzColour.z = xyzColour.y - b / 200.0;
        xyzColour.x = _xyzTransform(xyzColour.x);
        xyzColour.y = _xyzTransform(xyzColour.y);
        xyzColour.z = _xyzTransform(xyzColour.z);
        xyzColour.x *= 0.95047;
        xyzColour.z *= 1.08884;
        return xyzColour;
    };

    double CIEDE2000(const ColourSpaces::LAB& lab1, const ColourSpaces::LAB& lab2) {
        const double& l1 = lab1.l;
        const double& l2 = lab2.l;
        const double& a1 = lab1.a;
        const double& a2 = lab2.a;
        const double& b1 = lab1.b;
        const double& b2 = lab2.b;
        double dL = l2 - l1;
        double avgL = (l1 + l2) / 2;
        double c1 = sqrt(pow(a1, 2) + pow(b1, 2));
        double c2 = sqrt(pow(a2, 2) + pow(b2, 2));
        double avgC = (c1 + c2) / 2;
        double CTerm = 1 - sqrt(pow(avgC, 7) / (pow(avgC, 7) + 6103515625));
        double aCorr1 = a1 + (a1 / 2) * CTerm;
        double aCorr2 = a2 + (a2 / 2) * CTerm;
        double cCorr1 = sqrt(pow(aCorr1, 2) + pow(b1, 2));
        double cCorr2 = sqrt(pow(aCorr2, 2) + pow(b2, 2));
        double deltaCCorr = cCorr2 - cCorr1;
        double avgCCorr = (cCorr1 + cCorr2) / 2;
        double h1 = 0;
        double h2 = 0;
        if ((b1 != 0) || (aCorr1 != 0)) {
            h1 = atan2(b1, aCorr1);
            if (h1 < 0) {
                h1 += 2 * M_PI;
            }
        }
        if ((b2 != 0) || (aCorr2 != 0)) {
            h2 = atan2(b2, aCorr2);
            if (h2 < 0) {
                h2 += 2 * M_PI;
            }
        }
        double dh = 0;
        if (abs(h1 - h2) <= M_PI) {
            dh = h2 - h1;
        }
        else if (h2 <= h1) {
            dh = h2 - h1 + 2 * M_PI;;
        }
        else {
            dh = h2 - h1 - 2 * M_PI;
        }
        double H = 0;
        if (abs(h1 - h2) <= M_PI) {
            H = (h1 + h2) / 2;
        }
        else if (h1 + h2 < 2 * M_PI) {
            H = (h1 + h2 + 2 * M_PI) / 2;
        }
        else {
            H = (h1 + h2 - 2 * M_PI) / 2;
        }
        double dH = 2 * sqrt(cCorr1 * cCorr2) * sin(dh / 2);
        double T = 1 - 0.17 * cos(H - M_PI / 6) + 0.24 * cos(2 * H) + 0.32 * cos(3 * H + M_PI * 6 / 180) - 0.2 * cos(4 * H - M_PI * 63 / 180);
        double Sl = 1 + 0.015 * pow(avgL - 50, 2) / sqrt(20 + pow(avgL - 50, 2));
        double Sc = 1 + 0.045 * avgCCorr;
        double Sh = 1 + 0.015 * avgCCorr * T;
        double R = -2 * sqrt(pow(avgCCorr, 7) / (pow(avgCCorr, 7) + 6103515625)) * sin((M_PI / 3) * exp(-pow((H - 275*M_PI/180)/(25 * M_PI / 180), 2)));
        double corrTerm = R * deltaCCorr * dH / (Sc * Sh);
        return sqrt(pow(dL / Sl, 2) + pow(deltaCCorr / Sc, 2) + pow(dH / Sh, 2) + corrTerm);
    }
}

#endif