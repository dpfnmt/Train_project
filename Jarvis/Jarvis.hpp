#pragma once

#include<map>
#include<cstdio>
#include<iostream>
#include<string>
#include<sstream>
#include<memory>
#include<fstream>
#include<unistd.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<unordered_map>
#include<pthread.h>
#include<json/json.h>
#include "base/http.h"
#include"speech.h"
#include<sys/types.h>
#include<sys/stat.h>

using namespace std;

#define ASR_PATH  "temp_file/asr.wav"
#define TTS_PATH  "temp_file/tts.wav"
#define CMD_ETC  "command.etc"
#define LOG "log.txt"

class  Util{
	public:
		static bool Exec(std::string command, bool is_print) 
		{
			if(!is_print){
				command += ">/dev/null 2>&1";
			}
			FILE *fp = popen(command.c_str(), "r");
			if(nullptr == fp){
				std::cerr << "popen exec \' "<< command << "\' Error" << std::endl;
				return false;
			}
			if(is_print){
				char ch;
				while(fread(&ch, 1, 1, fp) > 0){
				fwrite(&ch, 1, 1, stdout);
				}
			}
			pclose(fp);
			return true;
		}
		static void* ThreadRun(void* arg)
		{
			const char* tips = (char*)arg;
			int i=0;
			char bar[103] = {0};
			const char*lable = "|/-\\";
			for(;i <= 50;i++){
					printf("%s[%-51s][%d%%][%c]\r",tips,bar,i*2,lable[i%4]);
					fflush(stdout);
					bar[i] = '=';
					bar[i+1] = '>';
					usleep(49000*2);
			}
			printf("\n");
		}
		/*static void PrintStart(std::string tips)
		{
			pthread_create(&id, NULL, ThreadRun,(void*)tips.c_str());
		}
       static void PrintEnd()	
	   { 	  
			pthread_cancel(id);
	   }
private:
	   static pthread_t  id;*/
	    static void PrintProcess()   //进度条
	   	{
			pthread_t tid;
			if(pthread_create(&tid,nullptr,ThreadRun,nullptr)!=0)
			std::cerr<<__TIME__<<"Thread creadte failed."<<std::endl;
	    }
};
 //pthread_t  Util::id;

class Robot{
	private:
		std::string url;
		std::string api_key;
		std::string user_id;
		aip::HttpClient client;

	private:
		bool IsCodeLegal(int code)
		{
			bool result = false;
			switch(code){
			case 5000:
			case 6000:
				break;
			case 10004:
				result = true;
				break;
			default:
				result = true;
				break;
			}
			return result;
		}
		std::string MessageToJson(std::string &message)
		{
		 	Json::Value root;
			Json::StreamWriterBuilder wb;
			std::ostringstream ss;
			Json::Value item_;
			item_["text"] = message;

			Json::Value item;
			item["inputText"] = item_;

			root["reqType"] = 0; // test
			root["perception"] = item;

			item.clear();
			item["apiKey"] = api_key;
			item["userId"] = user_id;
			root["userInfo"] = item;
			
			std::unique_ptr<Json::StreamWriter> sw(wb.newStreamWriter());
			sw->write(root,&ss);
			std::string json_string = ss.str();
		//	std::cout<< " debug: " << json_string << std::endl; 
			return json_string;
		}  
		std::string  RequestTL(std::string &request_json)
		{
			std::string response;
			int status_code  =	client.post(url , nullptr , request_json , nullptr ,&response);
			if(status_code != CURLcode::CURLE_OK)
			{
			 		std::cerr << "post error" << std::endl;
					return "";
			}
			return response;
		}
        std::string JsonToEchoMessage(std::string &str)
		{
		 		//std::cout<<"JsonToMessage: " << str << std::endl;
				JSONCPP_STRING errs;
				Json::Value root;
				Json::CharReaderBuilder rb;
				std::unique_ptr<Json::CharReader> const cr(rb.newCharReader());

				bool res = cr->parse(str.data(),str.data()+str.size(),&root,&errs);
				if(!res || !errs.empty())
				{
				 	std::cerr << "parse error!"<< std::endl;
					return "";
				}
				int code = root["intent"]["code"].asInt();
				if(!IsCodeLegal(code))
				{
				  std::cerr << "response code error" << std::endl;
				  return "";
				}
				Json::Value item = root["results"][0];
				std::string meg = item["values"]["text"].asString();
				return meg;

		}

	public:
		Robot(std::string  id = "1"){
			 this->url = "http://openapi.tuling123.com/openapi/api/v2";    
			 this->api_key = "23c3d6ff77bb4fd5b903a10e67c4799e";
			 this->user_id  = id; 
		}
		std::string Talk(std::string message)
		{
		   std::string json =  MessageToJson(message);
		   std::string response = RequestTL(json);
		   std::string echo_message = JsonToEchoMessage(response);
			return echo_message;
		}
	//	~Robot{
	// }
};

class SpeechRec{
private:
		std::string app_id;
		std::string api_key;
		std::string secret_key;
		aip::Speech * client;
      private: 
           bool IsCodeLegal(int code)
            {    
                 bool result = false;
	              switch(code){
	               case 0:
				   		result = true;
				   		break;
	              default:
			              break;
			      }
		             return result;
				} 
public:	
		SpeechRec()
		{
			app_id = "16882144";
			api_key = "ob1sfo4vlN7yIjH88wUE0Fdj";
			secret_key = "FzRxAEEq68qLOiADT5toSILtUZEUWWa9";
	 		client = new aip::Speech(app_id, api_key, secret_key);
		}
		bool  ASR(std::string path , std::string &out){
			std::map<std::string, std::string> options;
			options["dev_pid"] = "1536"; 
			std::string file_content;
			aip::get_file_content(ASR_PATH, &file_content);
			Json::Value result = client->recognize(file_content, "wav" , 16000, options);
		//	std::cout<< "debug: " << result.toStyledString() <<std::endl;//error
			int code = result["err_no"].asInt();
			if(!IsCodeLegal(code)){
				std::cerr << "recognize error " << std::endl;
				return false;
			}
			out = result["result"][0].asString();
			return true;
		}
		bool TTS(std::string message)
     	{
			bool ret;
			std::ofstream ofile;
			std::string ret_file;
			std::map<std::string , std::string> options;
			options["sspd"] = "8"; //语速 0~15
			options["pit"] = "8";   //音调 0~15
			options["vol"] = "10";  //音量 0~15
			options["per"] = "111";  //1,0,3,4,106,110,111,103,5
			options["aue"] = "6";    //格式，选择这里的话，宏定义那里要改变
			ofile.open(TTS_PATH, std::ios::out | std::ios::binary);

		Json::Value result = client->text2audio(message, options, ret_file);
		if(!ret_file.empty())
		{
			ofile << ret_file;
			//ofile.close();
			ret = true;
		}
		else
		{
			std::cerr << result.toStyledString() << std::endl;
			return false;
		}
		ofile.close();
		return ret;
		}

		~SpeechRec(){
		}
//private:           
//        std::string app_id;
//         std::string api_key;
//         std::string secret_key;
//		 aip::Speech * client;  
};

class Jarvis{
private:
		Robot rt;
		SpeechRec sr;
		std::unordered_map<std::string,std::string> commands;
private:
		bool Record()
		{
    	//	static Util::PrintStart(sr) ;
			std::cout << "debug: " << "Record....." << std::endl;
			std::string command = "arecord -t wav -c 1 -r 16000 -d 5 -f S16_LE ";
			command += ASR_PATH;
			bool ret =  Util::Exec(command, false);
		//	static Util::PrintEnd();
			std::cout << "debug: Record ... done" << std::endl;
			return ret;
		}
		bool Play()
		{
			std::string command = "cvlc --play-and-exit ";//空格
			command += TTS_PATH;
			bool ret = Util::Exec(command, false);
			return ret;
		}
public:
	Jarvis()
	{	    
	}
	bool LoadEtc()
	{
		char line[256];	
		std::ifstream in(CMD_ETC);
		if(!in.is_open()){
			std::cerr << "open error" << std::endl;
			return false;
		}
		std::string sep = ":";
		while(in.getline(line, sizeof(line))){
			std::string str = line;
			std::size_t pos = str.find(sep);
			if(std::string::npos == pos){
			//	std::cerr << "not find :  " <<std::endl;
				continue;
			}
			std::string k = str.substr(0,pos);
			std::string v = str.substr(pos+sep.size());
			k += "。";
			commands.insert(std::make_pair(k, v));
		}
		std::cerr << "Load command etc done ... success" <<std::endl;
		in.close();
		return true;
	}
	bool IsCommand(std::string message,std::string &cmd)
	{
		auto iter = commands.find(message);
		if(iter == commands.end()){
			return false;
		}
		cmd = iter->second;
	}
	void Run()
	{
#ifdef _LOG_	
		int fd = open(LOG, O_WRONLY | O_CREAT,0644);
		dup2(fd , 2);
#endif
		volatile bool quit = false;
		while(!quit){
			if(this->Record()){
				std::string message;
				if(sr.ASR(ASR_PATH, message))
				{
				 	//1.command
					std::string cmd;
					if(IsCommand(message, cmd)){
						std::cout<< "[Jarvis@localhost]$ "<< cmd << std::endl;

						Util::Exec(cmd, true);
						continue;
					}
					Util::PrintProcess();
					std::cout << "我# " << message <<std::endl;
					if(message == "你走吧。"){
						std::string quit_message = "那我就走了，不要想我哦！";
						
						 std::cout << "Jarvis# " << quit_message << std::endl;  
						if(sr.TTS(quit_message)){ 
						this->Play(); 
						}
						exit(0);
					}
					//Tuling
					//std::cout << "我# " << message <<std::endl;
					std::string echo = rt.Talk(message);
					std::cout <<"图灵机器人# "<< echo << std::endl;
					if(sr.TTS(echo)){
						this->Play();
					}
				//	std::cout <<"图灵机器人# "<< echo << std::endl;
				}
			}
			else{
				std::cerr << "Record error " << std::endl;
			}
		}
#ifdef _LOG_ 
		close(fd);
#endif
	}
	~Jarvis()
	{
	}
};




