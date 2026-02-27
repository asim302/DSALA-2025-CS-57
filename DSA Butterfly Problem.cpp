//2025(S)-CS-57

#include <iostream>
using namespace std;

int main() {
cout << "Problem 1\n";
    for(int row = 1; row <= 5; row++) {        
        for(int col = 1; col <= 6; col++) {   

            if(col == 1 || col == 6)          
                cout << "*";
                
            else if((row == 2 || row == 4) && (col == 2 || col == 5))
                cout << "*"; 

            else if ((row == 3) && (col == 3 || col == 4))
                cout << "*";    

            else
                cout << " ";                  
        }
        cout << endl;  
    }

    return 0;
}