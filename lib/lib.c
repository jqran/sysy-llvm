#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>
#include<stdio.h>
#include<stdarg.h>
#include<sys/time.h>
/* Input & output functions */
int64_t geti64(){int64_t t; scanf("%ld",&t); return t; }
int geti32(){int32_t t; scanf("%d",&t); return t; }
int16_t geti16(){int16_t t; scanf("%hu",&t); return t; }
int8_t geti8(){int8_t c; scanf("%c",&c);return c; }
int8_t getch(){return getchar(); }

float getf32(){
    float n;
    scanf("%a", &n);
    return n;
}
float getf64(){
    double n;
    scanf("%la", &n);
    return n;
}

int getarray(int a[]){
  int n;
  scanf("%d",&n);
  for(int i=0;i<n;i++)scanf("%d",&a[i]);
  return n;
}

int getfarray(float a[]) {
    int n;
    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        scanf("%a", &a[i]);
    }
    return n;
}
void puti64(int64_t a){ printf("%ld",a);}
void puti32(int32_t a){ printf("%d",a);}
void puti16(int16_t a){ printf("%d",a);}
void puti8(int8_t a){ printf("%c",a); }
void putarray(int n,int a[]){
  printf("%d:",n);
  for(int i=0;i<n;i++)printf(" %d",a[i]);
  printf("\n");
}
void putf32(float a) {
  printf("%a", a);
}
void putf64(double a) {
    printf("%lf", a);
  }
void putfarray(int n, float a[]) {
    printf("%d:", n);
    for (int i = 0; i < n; i++) {
        printf(" %a", a[i]);
    }
    printf("\n");
}

void putf(char a[], ...) {
    va_list args;
    va_start(args, a);
    vfprintf(stdout, a, args);
    va_end(args);
}
#ifdef __cplusplus
}
#endif