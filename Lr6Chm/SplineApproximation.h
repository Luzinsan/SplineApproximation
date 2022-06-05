#ifndef SPLINE_APPROZIMATION_H
#define SPLINE_APPROZIMATION_H
#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "Matrix.h"
#include "Vector.h"
#include "PolStr.h"
#include "Polynomial.h"

namespace luMath
{
    std::streambuf* redirectInput(std::ifstream* fin = NULL);
    std::streambuf* redirectOutput(std::ofstream* fout = NULL);
   
    char getSymbol(std::initializer_list<char> list,
        std::string notification_message = "",
        std::string error_message = "������������ ��������, ���������� ��� ���.\n-> ");
    double getDouble(double min = -DBL_MAX,
        double max = DBL_MAX,
        std::string notification_message = "",
        std::string error_message = "������������ ��������, ���������� ��� ���.\n-> ");
    template<class T>  Vector<T>& getGridX(Vector<T>& array, size_t size,
        std::string notification_message = "",
        std::string error_message = "������������ ��������, ���������� ��� ���.");
        
    template<class T>
    class SplineApproximation 
    {
    private:
        std::streambuf* _original_cin;
        std::streambuf* _original_cout;
        unsigned _k;   //  ������� ������� 
                       // (1 � ��������, 
                       //  2 � ��������������,
                       //  3 � ����������)
        unsigned _n;   // ���������� ��������;
        Vector<T> _x0; // ���� �����;
        Vector<T> _y0; // �������� ������� � ����� �����;
        T _0, _i;      // ��������� �������  (��� _k=2,3)
        unsigned _m;   // ���������� ���������� � �������������� �����
                       // (�.�.���������� ����� � m + 1,
                       //  ��� ������� ��� ���������� � ������ �������� �����);
        Vector<T> _res_x; // ���� �������������� �����;
        char _t;          // ������, ����������, �������� ��� ��� ������������� ��������� ��� ������� f(x)
                          // (y - ������������� ��������� ��������,
                          //  n - ������������� ��������� ����������);
        char* _f;         // ������������� ��������� ��� ������� (���� ��� ��������).
        Vector<Polynomial<T>> _splines;
    public:
        SplineApproximation()
            : _original_cin{ std::cin.rdbuf() }, _original_cout{ std::cout.rdbuf() },
            _k(1), _n(-1), _0(0), _i(0),
            _x0(), _y0(), _m(0),
            _res_x(), _t('n'), _f(NULL){}
        // ��������� ������ �����
        std::ifstream* setInputDevice(char input_method)
        {
            std::ifstream* fin = NULL;
            std::ofstream* fout = NULL;
            switch (input_method)
            {
            case '1': return NULL;
            case '2':
            {
                std::string filename;
                std::cout << "\n\t������� ��� �������� �����:\n-> ";
                getline(std::cin, filename);
                fin = new std::ifstream(filename);
                // ��������� ������ ����� ����� � �������������� ��� �� ���������������� ����
                _original_cin = redirectInput(fin);
                if (!_original_cin)  return NULL;

                std::cout << "\n\t������� ��� ��������� �����:\n-> ";
                getline(std::cin, filename);
                fout = new std::ofstream(filename);
                // ��������� ������ ����� ������ � �������������� ��� �� ���������������� ����
                _original_cout = redirectOutput(fout);
                if (!_original_cout)  return NULL;
                break;
            }
            case '3': case '4': case '5':
                if (input_method == '3')
                    fin = new std::ifstream("input_order_1.txt");
                else if (input_method == '4')
                    fin = new std::ifstream("input_order_2.txt");
                else
                    fin = new std::ifstream("input_order_3.txt");
                // ��������� ������ ����� ����� � �������������� ��� �� ���� input_non-uniform_grid.txt 
                _original_cin = redirectInput(fin);
                if (!_original_cin) return NULL;

                fout = new std::ofstream("output.txt", std::ios::app);
                // ��������� ������ ����� ������ � �������������� ��� �� ���� output.txt
                _original_cout = redirectOutput(fout);
                if (!_original_cout)  return NULL;
                break;
            default:
                throw std::invalid_argument("\n\t\t��� ����������� ������ ����� ������...\n");
            }
            return fin;
        }
        // ���������� ������ �� �������� ������ �����
        void inputData(std::ifstream* in)
        {
            if (!in)
            {
                _k = getSymbol({ '0','1','2' }, "\n\t������� ��������� ������� �������:"
                    "\n\t1 � ��������;"
                    "\n\t2 � ��������������;"
                    "\n\t3 � ����������.\n-> ") - '0';
                _n = static_cast<unsigned>(getDouble(0, INT_MAX, "\n\t������� ���������� ��������:\n-> "));
                _x0 = Vector<T>(_n + 1, false);
                _x0 = getGridX(_x0, _n + 1,
                        "\n\t������� �������� ����� ���������������� �����:\n",
                        "\n\t�������� ����� ������ ���� ������ �� �����������. ������� ������ ��������.");
                std::cout << "\n\t������� �������� ������� � ����� ���������������� �����:\n";
                _y0 = Vector<T>(_n + 1, false);
                for (unsigned i = 0; i <= _n; i++)
                    _y0[i] = getDouble(-DBL_MAX, DBL_MAX, (std::stringstream() << "-> [" << _x0[i] << "]: ").str()); 
                _m = static_cast<unsigned>(getDouble(0, INT_MAX, "\n\t������� ���������� ���������� � �������������� �����:\n-> "));
                _res_x = Vector<T>(_m + 1, false);
                _res_x =  getGridX(_res_x, _m + 1,
                        "\n\t������� �������� ����� �������������� ���������������� �����:\n",
                        "\n\t\t\t�������� ����� ������ ���� ������ �� �����������. ������� ������ ��������.\n");
                if (_k == 2)
                {
                    _0 = static_cast<unsigned>(getDouble(0, INT_MAX, "\n\t������� ������ ���������� �������:\n-> "));
                    _i = static_cast<unsigned>(getDouble(0, DBL_MAX, "\n\t������� �������� ���������� �������:\n-> "));
                }
                else if (_k == 3) 
                {
                    _0 = static_cast<unsigned>(getDouble(0, INT_MAX, "\n\t������� ������ ��������� �������:\n-> "));
                    _i = static_cast<unsigned>(getDouble(0, DBL_MAX, "\n\t������� ������ ��������� �������:\n-> "));
                }
                _t = getSymbol({ 'y', 'n' }, "\n\t�������� �� ������������� ��������� ��� ������� f(x)?"
                    "\n\ty � ��;"
                    "\n\tn � ���\n-> ");
                if (_t == 'y')
                {
                    std::string F;
                    char choice = 'y';
                    while (choice == 'y')
                    {
                        std::cout << "\n\t������� ��������� ��� �������:\n-> ";
                        getline(std::cin, F);
                        if (!F.empty())
                        {
                            _f = CreatePolStr(F.c_str(), 0);
                            if (GetError() != ERR_OK)
                            {
                                std::cerr << "\n\t������ ��������� �� ��������������.";
                                choice = getSymbol({ 'y','n' }, "\n\t����������� ��� ���? (y/n)\n-> ");
                            }
                            else choice = 'n';
                        }
                        else
                        {
                            std::cerr << "\n\t������ ���������� ������ ������.";
                            choice = getSymbol({ 'y','n' }, "\n\t����������� ��� ���? (y/n)\n-> ");
                        }
                    }
                }
                if (in)
                {
                    std::cin.rdbuf(_original_cin); // ���������� ����� �� ������������ ������������������ �����
                    in->close();
                }
            }
            else
            {
                std::cin  >> _k >> _n;
                _x0 = Vector<T>(_n + 1, false);
                for (unsigned i = 0; i <= _n; i++)
                    std::cin >> _x0[i];
                _y0 = Vector<T>(_n + 1, false);
                for (unsigned i = 0; i <= _n; i++)
                    std::cin >> _y0[i];
                std::cin >> _0 >> _i >>_m;
                _res_x = Vector<T>(_m + 1, false);
                for (unsigned i = 0; i <= _m; i++)
                    std::cin >> _res_x[i];
                std::cin >> _t;
                if (_t == 'y')
                {
                    std::cin.seekg(2, std::ios_base::cur);
                    std::string F;
                    getline(std::cin, F);
                    _f = CreatePolStr(F.c_str(), 0);
                    if (GetError() != ERR_OK)
                        std::cout << "\n\t����������� ������ ������������� ������� (��� �� �������� ���������). \n";
                }
            }
        }
        ~SplineApproximation()
        {
            if (_original_cout)
                std::cout.rdbuf(_original_cout); // ���������� �� ������������ ����� � ����������
            if (_f) { delete[] _f; _f = NULL; }
        }

        // ���������� ������� ��������
        unsigned getSplOrd() const { return _k; } 
        // ���������� ���������� ��������
        unsigned getCountSpl() const { return _n; }
        // ���������� ���� �������� �����
        const Vector<T>& getSourceGrid() const { return _x0;}
        // ���������� �������� ������� � ����� �������� �����
        const Vector<T>& getValueGrid() const { return _y0; }
        // ���������� ���������� ���������� � �������������� �����
        unsigned getCountRes() const { return _m; }
        const Vector<T>& getResultGrid() const { return _res_x; }
        char* getOrigAnalytic() const { return _f; }


        const Vector<Polynomial<T>>& getSplines(unsigned ord)
        {
            switch (ord)
            {
            case 1: // �������� �������
                std::cout << "\n\tOrder: 1.\n";
                _splines = SplineFirstOrd(_x0, _y0);
                break;
            case 2: // �������������� �������
            {   std::cout << "\n\tOrder: 2.\n";
                _splines = SplineSecondOrd(_x0, _y0, _0, _i);
                break;
            }
            case 3: // ���������� �������
                std::cout << "\n\tOrder: 3.\n";
                _splines = SplineThirdOrd(_x0, _y0, _0, _i, _original_cout);
                break;
            default: throw std::logic_error("�������������� ������� ��������");
            }
            return  _splines;
        }

        private:
            
            static Vector<Polynomial<T>> SplineFirstOrd(const Vector<T>& x, const Vector<T>& y)
            {
                size_t n = x.getLength() - 1;
                Vector<Polynomial<T>> Splines(n);
                
                for (int i = 0; i < n; i++)
                {
                    Splines[i] = y[i]
                        + ((y[i + 1] - y[i])
                            / (x[i + 1] - x[i])
                            * Polynomial<T>({ -(T)x[i], 1 }));
                    std::cout << "\nSpline #" << i << ":\t" << Splines[i];
                }
                return Splines;
            }
            static Vector<Polynomial<T>> SplineSecondOrd(const Vector<T>& x, const Vector<T>& y, int index, T border)
            {
                size_t n = x.getLength() - 1;
                Vector<Polynomial<T>> Splines(n);
                Vector<T> b(n + 1);
                if (index == 0)
                {
                    b[0] = border;
                    for (int i = 0; i < n; i++)
                        b[i + 1] = 2 * (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - b[i];
                }
                else
                {
                    b[n] = border;
                    for (int i = n - 1; i >= 0; i--)
                        b[i] = 2 * (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - b[i + 1];
                }
                for (int i = 0; i < (int)n; i++)
                {
                    Splines[i] = y[i]
                        + (b[i] * Polynomial<T>({ -(T)x[i], 1 }))
                        + (((b[i + 1] - b[i])
                            / (2 * (x[i + 1] - x[i]))
                            * Polynomial<T>({ -(T)x[i], 1 }) * Polynomial<T>({ -(T)x[i], 1 })));
                    std::cout << "\nSpline #" << i << ":\t" << Splines[i];
                }
                return Splines;
            }

            static Vector<Polynomial<T>> SplineThirdOrd(const Vector<T>& x, const Vector<T>& y, T left, T right, std::streambuf* out)
            {
                size_t n = x.getLength();
                Vector<Polynomial<T>> Splines(n - 1);
                Matrix<T> A(n);
                A[0][0] = (x[1] - x[0]) / 3;

                Vector<T> g(n);
                g[0] = (y[1] - y[0]) / (x[1] - x[0]) - left;
                for (int i = 1; i < n - 1; i++)
                {
                    A[i][i] = (x[i] - x[i - 1] + x[i + 1] - x[i]) / 3;
                    A[i - 1][i] = A[i][i - 1] = (x[i] - x[i - 1] ) / 6;
                    g[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
                }
                A[n - 1][n - 1] = (x[n - 1] - x[n - 2]) / 3;
                A[n - 2][n - 1] = A[n - 1][n - 2] = (x[n - 1] - x[n - 2]) / 6;
                g[n - 1] = right - (y[n - 1] - y[n - 2]) / (x[n - 1] - x[n - 2]);

               
                
                Vector<T> M = SweepMethod(A, g, out);
                std::cout << "\n\tM: " << M << "\n";
                for (int i = 0; i < n - 1; i++)
                {
                    Splines[i] = y[i]
                        + ((y[i + 1] - y[i]) / (x[i + 1] - x[i])
                            - (x[i + 1] - x[i]) * (2 * M[i] + M[i + 1]) / 6
                            * Polynomial<T>({ -(T)x[i], 1 }))
                        + (M[i] / 2
                            * Polynomial<T>({ -(T)x[i], 1 }) * Polynomial<T>({ -(T)x[i], 1 }))
                        + ((M[i + 1] - M[i]) / (x[i + 1] - x[i]) / 6
                            * Polynomial<T>({ -(T)x[i], 1 }) * Polynomial<T>({ -(T)x[i], 1 }) * Polynomial<T>({ -(T)x[i], 1 }));
                    std::cout << "\nSpline #" << i << ":\t" << Splines[i];
                }
                return Splines;
            }

            static Vector<T> SweepMethod(Matrix<T> A, const Vector<T> b, std::streambuf* out)
            {
                size_t n = b.getLength();
                Vector<T> x(n);
                Vector<T> r(n);
                r[0] = -A[0][1] / A[0][0];
                
                Vector<T> v(n);
                v[0] = b[0] / A[0][0];
                
                for (int i = 1; i < n - 1; i++)
                {
                    r[i] = -A[i][i + 1] / ( A[i][i] + A[i][i - 1] * r[i - 1]);
                    v[i] = (b[i] - A[i][i - 1] * v[i - 1]) / (A[i][i] + A[i][i - 1] * r[i - 1]);
                }
                v[n - 1] = (b[n - 1] - A[n - 1][n - 2] * v[n - 2]) / (A[n - 1][n - 1] + A[n - 1][n - 2] * r[n - 2]);
                
                // �������� ���
                x[n - 1] = v[n - 1];
                for (int i = n - 2; i >= 0; i--)
                    x[i] = r[i] * x[i + 1] + v[i];
                return x;
            }

        /*void checkSourceGrid()
        {
            bool success = true;
            for (unsigned i = 0; i <= _n; i++)
            {
                std::cout << "\tP(" << _x0[(unsigned)i] << ") = " << pol(_x0[(unsigned)i]) << "\n";
                if (abs(pol(_x0[(unsigned)i]) - _y0[i]) > EPS)
                {
                    success = false;
                    std::cout << "\t f(" << _x0[(unsigned)i] << ") = " << _y0[i]
                        << "\n\t������� ������������ �� �����������. �����������: " << abs(pol(_x0[(unsigned)i]) - _y0[i]) << "\n";
                }
            }
            
            if (success) std::cout << "\n\t������ ���������������� ������� � ����� �������� ����� ��������� � �������� �������� �����, ������� ����������."
                << "\n������� ������������ ���������.";
        }

        void checkResGrid(Polynomial<T> pol)
        {
            for (unsigned i = 0; i <= _m; i++)
            {
                std::cout << "\tP(" << _res_x[(unsigned)i] << ") = " << pol(_res_x[(unsigned)i]) << "\n";
                if (_f)
                {
                    T res_f = EvalPolStr(_f, _res_x[(unsigned)i], _k);
                    std::cout << "\tf(" << _res_x[(unsigned)i] << ") = " << res_f
                        << "\n\t�����������: " << abs(pol(_res_x[(unsigned)i]) - res_f) << "\n";
                }
            }
        }*/
        /*
        friend std::ostream& operator<<(std::ostream& out, SplineApproximation& data)
        {
            std::streamsize precision = std::cout.precision();
            std::streamsize size = std::cout.width();
            std::cout << std::setprecision(precision);

            out << "\n\t�������� ������� �������� ������������ ���������������� �������: " << data.getPolOrder()
                << "\n\t��� �������� �����: ";
            if (data.getGridType() == 'u')
                out << "����������� �����.\n";
            else out << "������������� �����.\n";
            out << "��������� �����: " << "\t\t\t\t\t\t" << std::setw(size) << data.getSourceGrid()
                << "��������������� �������� ������ �����: " << "\t" << std::setw(size) << data.getValueGrid()
                << "��������� ������� ����������� ��������������� �������: " << data.getDerivative();

            switch (data.getMethod())
            {
            case '1':
                out << "\n������� ������� � " << data.getDerivative() << " �������� �����������:\t"
                    << std::setw(size) << data.getNewtonInterPol(data.getDerivative())
                    << "\n������ �������� ��������������� ������� � ��������� �������� ����� � �������� �����: \n";
                data.checkSourceGrid(data.getNewtonInterPol(0));
                out << "\n�������� �������� ��������������� ������� ���������� ������� ����������� � �������������� �����:\n";
                data.checkResGrid(data.getNewtonInterPol(data.getDerivative()));

                break;
            case '2':
                out << "\n������� �������� � " << data.getDerivative() << " �������� �����������:\t"
                    << std::setw(size) << data.getLagrangeInterPol(data.getDerivative())
                    << "\n������ �������� ��������������� ������� � ��������� �������� ����� � �������� �����: \n";
                data.checkSourceGrid(data.getLagrangeInterPol(0));
                out << "\n�������� �������� ��������������� ������� ���������� ������� ����������� � �������������� �����:\n";
                data.checkResGrid(data.getLagrangeInterPol(data.getDerivative()));
                break;
            }
            return out;
        }
        */
    
    };

    std::streambuf* redirectInput(std::ifstream* in)
    {
        std::streambuf* original_cin = std::cin.rdbuf();
        while (!*in)
        {
            std::string filename;
            char choice;
            choice = getSymbol({ '1', '2' },
                "������ ���� �� ����� ���� ������, ���� �� ����������. ����������� ��� ���?\n1) ��\n2) �����\n->");
            if (choice == '1')
            {
                std::cout << "������� ��� �����:\n->";
                getline(std::cin, filename);
            }
            else return NULL;
            in->open(filename);
        }
        //�������������� ����������� ����� ����� �� ���������� ����
        std::cin.rdbuf(in->rdbuf());
        return original_cin;
    }

    std::streambuf* redirectOutput(std::ofstream* out)
    {
        std::streambuf* original_cout = std::cout.rdbuf();
        while (!*out)
        {
            std::string filename;
            char choice;
            choice = getSymbol({ '1', '2' },
                "������ ���� �� ����� ���� ������, ���� �� ����������. ����������� ��� ���?\n1) ��\n2) �����\n->");
            if (choice == '1')
            {
                std::cout << "������� ��� �����:\n->";
                getline(std::cin, filename);
            }
            else return NULL;
            out->open(filename);
        }
        //�������������� ����������� ����� ������ �� ���������� ����
        std::cout.rdbuf(out->rdbuf());
        return original_cout;
    }


    char getSymbol(std::initializer_list<char> list,
        std::string notification_message,
        std::string error_message)
    {
        char choice;
        std::cout << notification_message;

        bool flag = true;
        do {
            choice = _getche();
            std::cout << std::endl;
            for (auto it = list.begin(); it != list.end(); it++)
                if (it[0] == choice) { flag = false; break; }

            if (flag) std::cerr << error_message;
        } while (flag);
        return choice;
    }

    double getDouble(double min, double max,
        std::string notification_message,
        std::string error_message)
    {
        std::string epsstr;
        double eps;
        do {
            std::cout << notification_message;
            std::cin >> epsstr;

            bool point = false, flag = false;
            auto it = epsstr.begin();
            if (it[0] == '-') it++;
            for (; it != epsstr.end(); it++)
                if (!isdigit(it[0]) && (it[0] != ',' || point))
                {
                    std::cout << error_message;
                    flag = true;
                    break;
                }
                else if (it[0] == ',' && !point) point = true;

            if (flag) continue;
            eps = std::stod(epsstr);
            if (eps <= min || eps >= max)
                std::cout << error_message;
            else { std::cin.ignore(32256, '\n'); break; }
        } while (true);
        return eps;
    }

    template<class T> Vector<T>& getGridX(Vector<T>& array, size_t size,
        std::string notification_message,
        std::string error_message)
    {
        std::cout << notification_message;
        int i = 0;
        array[i] = getDouble(-DBL_MAX, DBL_MAX, (std::stringstream() << "-> [" << i << "]: ").str());
        i++;
        do
        {
            array[i] = getDouble(-DBL_MAX, DBL_MAX, (std::stringstream() << "-> [" << i << "]: ").str());
            if (array[i] < array[i - 1]) std::cout << error_message;
            else i++;
        } while (i < size);
        return array;
    }
}

#endif