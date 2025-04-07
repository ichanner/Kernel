
int min(int a, int b){

	if(a < b){

		return a;;
	}

	return b;
}

int max(int a, int b){

	if(a > b){

		return a;
	}

	return b;
}	

int ceil(double a){
    
    int truncated = a;
    double diff = a - truncated;
    
    if(diff != 0){
        
        return (truncated + 1);
    }
    
    return (int)a;

}
