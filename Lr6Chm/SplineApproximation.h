#ifndef SPLINE_APPROZIMATION_H
#define SPLINE_APPROZIMATION_H
#include <iostream>
#include <fstream>
#include <conio.h>
#include <string>
#include <sstream>
#include <iomanip>
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

                fout = new std::ofstream("output.txt");
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
            _splines = Vector<Polynomial<T>>(_n);
            switch (ord)
            {
            case 1: // �������� �������
                for (int i = 0; i < (int)_n; i++)
                    _splines[i] = _y0[i] + (_y0[i + 1] - _y0[i]) / (_x0[i + 1] - _x0[i]) * Polynomial<T>({ -(T)_x0[i], 1 });
                break;
            case 2: // �������������� �������
            {   
                Vector<T> b(_n + 1);
                if (_0 == 0)
                {
                    b[0] = _i;
                    for (int i = 0; i < (int)_n; i++)
                        b[i + 1] = 2 * (_y0[i + 1] - _y0[i]) / (_x0[i + 1] - _x0[i]) - b[i];
                }
                else
                {
                    b[_n] = _i;
                    for (int i = _n - 1; i >= 0; i--)
                        b[i] = 2 * (_y0[i + 1] - _y0[i]) / (_x0[i + 1] - _x0[i]) - b[i + 1];
                }

                for (int i = 0; i < (int)_n; i++)
                {
                    _splines[i] = _y0[i]
                        + b[i] * Polynomial<T>({ -(T)_x0[i], 1 })
                        + ((b[i + 1] - b[i])
                            / (2 * (_x0[i + 1] - _x0[i]))
                            * Polynomial<T>({ -(T)_x0[i], 1 }) * Polynomial<T>({ -(T)_x0[i], 1 }));
                    std::cout << "\nSpline #" << i << ":\t" << _splines[i];
                }
                break;
            }
            case 3: // ���������� �������
                //for (int i = 0; i <= (int)_n; i++)
                    //SplineThirdOrd(Splines[i]);
                break;
            default: throw std::logic_error("�������������� ������� ��������");
            }
            return  _splines;
        }

        private:
            
           
            /*const Polynomial<T>& SplineSecondOrd(Polynomial<T>& spline)
            {
                Polynomial<T> P;
                for (int i = 0; i <= (int)_n; i++)
                {
                    Polynomial<T> mult((T)1);
                    for (int j = 0; j <= i - 1; j++)
                    {
                        if (_s == 'u')
                            mult *= Polynomial<T>({ -(T)j, 1 });
                        else
                            mult *= Polynomial<T>({ -_x0[j], 1 });

                    }
                    if (_s == 'u')
                        P += dividedDifferences[i][0] * mult / factorial(i);
                    else
                        P += dividedDifferences[i][0] * mult;
                }
                return P;
            }
            
            const Polynomial<T>& SplineThirdOrd(Polynomial<T>& spline)
            {
                Polynomial<T> P;
                for (int i = 0; i <= (int)_n; i++)
                {
                    Polynomial<T> mult((T)1);
                    for (int j = 0; j <= i - 1; j++)
                    {
                        if (_s == 'u')
                            mult *= Polynomial<T>({ -(T)j, 1 });
                        else
                            mult *= Polynomial<T>({ -_x0[j], 1 });

                    }
                    if (_s == 'u')
                        P += dividedDifferences[i][0] * mult / factorial(i);
                    else
                        P += dividedDifferences[i][0] * mult;
                }
                return P;
            }*/


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