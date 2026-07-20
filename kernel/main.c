
void main(){
    /*
    summary: up to know entry.S and start.c is running in each cpu core indivitually
    but there are few things that we want to run only one by any of the cpu core, so we
    are using the cpuid() == 0
    */
    if (cpuid() == 0){
        userinit(); // start first user process
    }
}