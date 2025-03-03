
int ceil(double a){
    
    int truncated = a;
    double diff = a - truncated;
    
    if(diff != 0){
        
        return (truncated+1);
    }
    
    return (int)a;

}
