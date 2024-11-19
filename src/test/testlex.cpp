#include "frontend/lex.hpp"
#include <cstdint>
#include <iomanip>
#include <memory>
#include <fstream>
#include <iostream>
using namespace std;
int main(int argc , char**argv){
    ifstream ifs;
        ifs.open("../../../../testcase/src/main.cj",ios::in);

    //std::string some_str(begin, end);

    if (!ifs.is_open())
    {
        cout << "read fail." << endl;
        return -1;
    }
	string content( (istreambuf_iterator<char>(ifs) ),
					 (istreambuf_iterator<char>() ) );

    // char buf[1000] = { 0 };
    // // int a=0;
    // // while (ifs >> buf)
    // // {   a++;
    // //     cout << buf <<"\tline "<<a<<endl;
    // // }
	// cout << content << endl;
	ifs.close();
    Lexer lexTest{content};
    std::unique_ptr<Token> tok;
    cout<<"line"<<std::setw(8)<<"column"<<std::setw(8)<<"line"<<std::setw(8)<<"column"<<std::setw(8)<<"type"<<std::setw(8)<<"literal"<<endl;
    do{
        tok=lexTest.nextToken();
        cout<<tok->begin.line<<std::setw(8)<<tok->begin.column<<std::setw(8)<<tok->end.line<<std::setw(8)<<tok->end.column<<std::setw(8)<<(uint64_t)tok->type<<std::setw(8)<<tok->literal<<'\n';
    }while(tok->type!=tokenType::LEXEOF);
}