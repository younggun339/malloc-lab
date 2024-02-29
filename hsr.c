 #include <stdio.h> 

 int main() { 
    char* str[2]; 
    str[0] = "hello!"; 
    str[1] = "jungler"; 
    printf("1. %s\n", str[0] + 1); 
    printf("2. %s\n", (str + 1)[0] + 2); 
    return 0; 
 }