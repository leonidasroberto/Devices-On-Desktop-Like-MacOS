#include <iostream>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <dirent.h>
#include <set>
#include <algorithm>
///#include <array>
///#include <cstring>

using namespace std;

const string desktopname = "/Área\\ de\\ Trabalho/";
const string removibleicon = "drive-removable-usb";
const string harddiskicon = "drive-harddisk-system";

string replacer(string text)
{
    for(int i = 0; i < text.length(); i++)
    {
        if(text[i] == ' ')
            text[i] = '?';
    }
    return text;
}

string execute(string cmd) {

  string data;
  FILE * stream;
  const int max_buffer = 256;
  char buffer[max_buffer];
  cmd.append(" 2>&1");

  stream = popen(cmd.c_str(), "r");
  if (stream) {
    while (!feof(stream))
      if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
    pclose(stream);
  }
  return data;
}

void sleepp(int tempo){
	tempo = tempo * 1000;
	usleep(tempo);
}

set<string> getSaved(){
	ifstream file ("data");
	string line;
	set<string> buffer;
	if(file){
		while(getline(file, line)){
			buffer.insert(line);
		}
	}
	file.close();
	return buffer;

}

void inputSave(set<string> scan, bool mod){
	ofstream file ("data");

	if (mod){
		for(string line: scan){
			/// << "GRAVANDO NO ARQUIVO!!!";
			file << line << "\n";
		}
	} 
	else {
		file << "" << endl;
	}
	file.close();
}

string devicetype(string n){
	string cmd = "./getDevice ";
	string device = execute(cmd.append(n));
	device.erase(remove(device.begin(), device.end(), '\n'), device.end());
	return device;

}

void createIcon(string name, string mediaName, string deskName){
		if (name != ""){
			string command = "ln -s ";
			command.append(mediaName);
			command.append(name);
			command.append(" ");
			command.append(deskName);
			///cout << "COMAND -> " << command << "\n";
			system(command.c_str());
		}
 }
 
 void removeIcon(string name, string deskName){
	 	if(name != ""){
			string command = "rm ";
			command.append(deskName);
			command.append(name);
			system(command.c_str());
 		}
 }

void typeIcon(string name, bool tipo, string mediaName){
	mediaName.append(name);
	mediaName.append("/.directory");
	ofstream file (mediaName);
	
	if(tipo){
		file << "[Desktop Entry]\nIcon="+harddiskicon;
	}else{
		file << "[Desktop Entry]\nIcon="+removibleicon;
	}
	file.close();

}

string getDirname(int tipo){
	string userName = execute("echo $USER");
	string mediaName;
	
	switch(tipo){
		case 1:
			mediaName = "/home/";
			mediaName.append(userName);
			mediaName.append(desktopname);
			mediaName.erase(remove(mediaName.begin(), mediaName.end(), '\n'), mediaName.end());
			return mediaName;
		case 2:
			mediaName = "/media/";
			mediaName.append(userName);
			mediaName.append("/");
			mediaName.erase(remove(mediaName.begin(), mediaName.end(), '\n'), mediaName.end());
			return mediaName;
		default:
			return "NULL";

	}

}


set<string> scanner(string dname){
	DIR *d = opendir(dname.c_str());
	struct dirent *dir;
	set<string> buffer;

	while (dir = readdir(d)){
		string v = dir->d_name;
		///cout << v << '\n'; 

		if((v != ".") && (v != "..")){
			///buffer.push_back(dir->d_name);
			buffer.insert(replacer(dir->d_name));
		}
	}
	closedir(d);
	return buffer;
}

enum dirNames {
	DESKTOP=1,
	MEDIA=2
};


int main(){

	bool status = true;
	set<string> disk = {"ntfs", "ext4"};
	set<string> scan;
	set<string> created;
	set<string> saved = getSaved();
	///string mediaName = "/media/leonidas/";
	string mediaName = getDirname(dirNames::MEDIA);
	string deskName = getDirname(dirNames::DESKTOP);
	///DIR *d = opendir(getDirname().c_str());
	///system("ls /home/leonidas/Downloads");
	while(true){
		///cout << "mediaName -> " << mediaName << "\n";
		if(status){
			///cout << "CARREGANDO SALVO..." << "\n";
			///scan=getSaved();
		    scan = saved;
			status = false;	

		}else{
			scan=scanner(mediaName);
			////cout << scan;
		
		}

		if(scan != saved){
			saved=scan;
			if(scan.size() == 0){
				inputSave(scan, false);	
			}else {
				inputSave(scan, true);
			}
			
		} 

		///Verificar no diretório
		for(string item: scan){
			if(created.find(item) == created.end()){
				string typeDev = devicetype(item);
				///cout << "TIPO -> " << tipo << "\n";
				///if(disk.find(tipo) == disk.end()){
				if(find(disk.begin(), disk.end(), typeDev) != disk.end()){
					///cout << "É dicos rigido!" << "\n";
					typeIcon(item, true, mediaName);
				} else {
					///cout << "É pendrive!" << "\n";
					typeIcon(item, false, mediaName);
				}
				created.insert(item);
				createIcon(item, mediaName, deskName);
			}
			
		}

		///Verifica na memória
		vector<string> toRemove;
		for (string item: created){
			///cout << item << "<- Armazenado em memória\n";
			
			if(scan.find(item) == scan.end()){
				toRemove.push_back(item);
			}
			
		}

		////Removendo...
		for (string item: toRemove){
			created.erase(item);
			removeIcon(item, deskName);
		}

		sleepp(3000);
	}

	////cout << "OK";
	return 0;
}

