#include "Jarvis.hpp"
using namespace std;


int main()
{
//	Robot r;
//	string str;
//	volatile bool quit = false;
//	while(!quit){
//		cout << "我# ";
//		cin >> str;
//		std::string s = r.Talk(str);
//		std::cout<<"Tom$ " << s << std::endl;
//}
    Jarvis *js = new Jarvis();
	if(!js->LoadEtc()){
		return 1;
	}
    js->Run();
   return 0;
}
