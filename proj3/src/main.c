extern double f_1 (double);
extern double f_2 (double);
extern double f_3 (double);
#include <stdio.h>
#include <math.h>
#include <getopt.h>
#define max_counts 10000
#define left -4.02675
#define right -0.371819
#define aa -5
#define bb -0.1
#define eps 0.001

static void help(void){
    printf("%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n", "The --root and -r options, which print the abscissas of the intersection points of the curves", "The options --iterations and -i, which print the number of iterations required to approximate the solution of the equations when searching for intersection points.", "The --test-root and -R options, which allow you to test the root function. The actual parameters of the root call are set by the only parameter of this option in the form F1:F2:A:B:E:R, where F1, F2 are the numbers of the functions used, A, B, E are the values of the parameters a, b, eps1 of the root function, R is the correct answer (calculated analytically). The program should call the root function with the specified parameters, compare the result with the correct answer and display the result, absolute and relative error", "The --test-integral and -I options, which allow you to test the integral function. The actual parameters of the integral call are set by the only parameter of this option in the form F:A:B:E:R, where F are the numbers of the functions used, A, B, E are the values of the parameters a, b, eps2 of the integral function, R is the correct answer (calculated analytically). The program must call the integral function with the specified parameters, compare the result with the correct answer and display the result, the absolute and relative error", "A program running without input parameters should output the answer to the task (the area of the figure)");
}

int iter = 0;
static double chord_method(double f (double), double g (double), double a, double b, double e) {
    printf("%lf %lf", a, b);
	if (fabs(b - a) < 0.001*e && fabs(f(a) - g(a)) < 0.001*e) {		/* Случай, если границы совпадают. */
		return a;
	} else if (fabs(b - a) < 0.001*e) {
		return 1;
	}
    int iterations = 0;
	double x0 = a;
	double Fb = f(b) - g(b);
	double Fa = f(a) - g(a);
	if ((Fa * Fb) > 0.0) {
        return 1;}
	double c = (a * Fb - b * Fa) / (Fb - Fa);
	while (fabs(f(x0) - g(x0)) > (0.5 * e)) {
		x0 = c;

		if ((f(a) - g(a)) * (f(b) - g(b)) > 0.0) {
			return 10;
		}
		if (iterations > max_counts) {
			iterations = -1;
			return 20;
		}
		double F1 = f(a/2.0 + b/2.0) - g(a/2.0 + b/2.0);
		double F2 = (f(a) - g(a) + f(b) - g(b))/2.0;
		double F = f(a) - g(a);
		if (F * (F1 - F2) > 0.0) {
			a = c;
		} else {
			b = c;
		}

		Fb = f(b) - g(b);
		Fa = f(a) - g(a);
		c = (a * Fb - b * Fa) / (Fb - Fa);

		iterations++;
		iter++;
	}

	return x0;
}

static double integral(double f (double), double a, double b, double e) {
    int n = 1 / e;
    double h = (b - a) / n ;
	double s = 0.5*f(a)*h;
	a += h;
	int i = 1;
	while (i < n) {
		s += f(a) * h;
		a += h;
		i++;
	}
	s += 0.5 * f(b) * h;

	return s;
}

const struct option option_list[] = {
    {"help", no_argument, 0, 'h'},
    {"root", no_argument, 0, 'r'},
    {"iterations", no_argument, 0, 'i'},
    {"test-root", required_argument, 0, 'R'},
    {"test-integral", required_argument, 0, 'I'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[]){
    double S1 = integral(f_1, left, right, eps);
    double S2 = integral(f_2, left, right, eps);
    double S3 = integral(f_3, left, right, eps);
    double area = S1 - S2 - S3;

    //double r12 = chord_method(f_1, f_2, aa, bb, eps);
    //int i12 = iter;
    //double r13 = chord_method(f_1, f_3, aa, bb, eps);
    //int i13 = iter;
    //double r23 = chord_method(f_2, f_3, aa, bb, eps);
    //int i23 = iter;

int option = getopt_long(argc, argv, "hriR:I:", option_list, NULL);
int f1, f2;
double a, b, e, r;

double (*funcs[3])(double x) = {f_1, f_2, f_3};
switch (option){
case -1:
    printf("%lf\n", area);
    break;
case 'h':
    help();
    break;
case 'r':
    //printf("f1 = f2: %lf\nf1 = f3: %lf\nf2 = f3: %lf\n", r12, r13, r23);
    break;
case 'i':
    //printf("f1 = f2: %d\nf1 = f3: %d\nf2 = f3: %d\n", i12, i13, i23);
    break;
case 'R':
    sscanf(optarg, "%d:%d:%lf:%lf:%lf:%lf", &f1, &f2, &a, &b, &e, &r);
    double ans = chord_method(funcs[f1 - 1], funcs[f2 - 1], a, b, e);
    printf("\n%lf %lf %lf\n", ans, a, b);
    if (fabs(ans - 10.0) < e){
        printf("Error, there is no roots on [a; b]\n");
        break;
    }
    if (fabs(ans - 20.0) < e){
        printf("Error, big difference between a and b\n");
        break;
    }
    double abs_e = fabs(ans - r);
    double otn_e = abs_e / ans;
    printf("%lf %lf %lf\n", ans, abs_e, otn_e);
    break;
case 'I':
    sscanf(optarg, "%d:%lf:%lf:%lf:%lf", &f1, &a, &b, &e, &r);
    double ians = integral(funcs[f1 - 1], a, b, e);
    double iabs_e = fabs(ians - r);
    double iotn_e = iabs_e / ians;
    printf("%lf %lf %lf\n", ians, iabs_e, iotn_e);
    break;
default:
    break;
    return 0;
}}
