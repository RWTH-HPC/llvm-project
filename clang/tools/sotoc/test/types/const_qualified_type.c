// RUN: %sotoc-transform-compile

int main() {
  const double cnum = 17.0;
  double num = 0.0;

  #pragma omp target map(to: cnum) map(tofrom: num)
  for (int i = 0; i < 10; ++i) {
    num += cnum;
  }
}