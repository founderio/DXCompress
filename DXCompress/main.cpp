	//
	//  main.cpp
	//  DXCompress
	//
	//  Created by Oliver Kahrmann on 08.05.11.
	//  Copyright 2011 DHELIXSoft. All rights reserved.
	//

#include <iostream>
#include <string>
#include <fstream>
#include <math.h>
#include <deque>
#include <stack>

using namespace std;

class area {
	long pos;
	long length;
};

void compress()
{
	cout << "Filename to compress: ";
		//Startvariablen
	string filename;
	fstream compressfile;
	long datalength;
	char *tempdata;
	
	fstream output;
	
		//Kompressorvariablen
	stack<long> matches;
	long pruefblocklaenge;
	long pruefposition;
	long pruefblockposition;
	string data;
	
		//Zu komprimierende Datei öffnen
	cin >> filename;
	
	compressfile.open(filename.c_str(), ios::in);
	
	
		//Daten in Speicher laden
	compressfile.seekg(0, ios::end);
	datalength = compressfile.tellg();
	datalength++;
	compressfile.seekg(0, ios::beg);
	
	if(datalength < 10)
	{
		cout << "The file is shorter than 10 characters. There is no value for compression." << endl;
		return;
	}
	
	tempdata = new char[datalength];
	compressfile.read(tempdata, datalength);
	data.append(tempdata);
	
		//Zu komprimierende Daten werden nicht mehr gebraucht, also schließen.
	compressfile.close();
	
		//Ausgabedatei öffnen
	output.open("compressed.txt", ios::out);
	
		//Prüfblocklänge auf die Hälfte der Datei setzen
	pruefblocklaenge = floor(datalength/2);
	if(pruefblocklaenge > 100)
	{
		pruefblocklaenge = 100;
	}
	
	
		//cout << data;
	
	while (true) {
		
			//Prüfblock auf Start setzen
		pruefblockposition = 0;
		
			//Statusausgabe
		cout << "Datalength: " << data.length() << ", ";
		cout << "Checkblocklength: " << pruefblocklaenge << endl;
		
		while ((pruefblocklaenge+pruefblockposition) <= (data.length()-pruefblocklaenge)) {
				//Prüfposition direkt hinter den Prüfblock setzen, vorher haben wir ja schon in vorherigen Durchläufen getestet.
			pruefposition = pruefblocklaenge + pruefblockposition;
			
			
			
				//Indikator, ob eine Übereinstimmung gefunden wurde
			bool hatWasGepasst = false;
			
			while (pruefposition+pruefblocklaenge <= data.length()) {
					//Prüfblock und Prüfposition gleich?
				if (data.compare(pruefblockposition, pruefblocklaenge, data, pruefposition, pruefblocklaenge) == 0) {
						//Ja, also Adresse merken
					matches.push(pruefposition);
						//Und den Indikator auf true setzen
					hatWasGepasst = true;
						//Jeztzt können wir den Rest der gefundenen Übereinstimmung überspringen, die wird ja später gelöscht.
					pruefposition+= pruefblocklaenge;
				} else {
						//Nein, also weiter bei der nächsten Position
					pruefposition++;
				}
			}
				//Gab es eine Übereinstimmung?
			if(hatWasGepasst)
			{
					//Berechnung, ob es eine Speicherersparnis gibt
				unsigned long length = pruefblocklaenge + sizeof(int)*(3 + matches.size());
				
				unsigned long lengthbefore = (matches.size() + 1) * pruefblocklaenge;
				
					//Statusausgabe
				cout << "Compression matches: " << matches.size() << endl;
				cout << "Length before: " << lengthbefore << ", length after: " << length << endl;
				
					//Speicherersparnis?
				if(length < lengthbefore)
				{
						//Ja, also gefundene Blöcke aus den Daten entfernen (rückwärts)
					long pos;
					while (!matches.empty()) {
							//Nächsten Block holen
						pos = matches.top();
						matches.pop();
							//Entpackanweisung in Binärform in die Kommandodatei schreiben
						output.write((char *)&pos, sizeof(int));
							//Datenblock löschen
						data.erase(pos, pruefblocklaenge);
					}
						//Anweisungen, welche Daten gepackt wurden in die Kommandodatei schreiben
					output.write((char *)&pruefblockposition, sizeof(int));
					output.write((char *)&pruefblocklaenge, sizeof(int));
						//Trennzeichen schreiben, um von nächsten Datenpaaren zu trennen
					int delim = -1;
					output.write((char *)&delim, sizeof(int));
				} else {
						//Nein, also die gefundenen Datenpaare aus der Arbeitsliste entfernen
					while (!matches.empty()) {
						matches.pop();
					}
					cout << "No value, I won't do it." << endl;
				}
			}
			hatWasGepasst = false;
				//Prüfblock verschieben
			pruefblockposition++;
		}
			//Weiter verschieben geht nicht, also machen wir den Prüfblock eins kleiner.
		pruefblocklaenge--;
			//Ist die Länge des Prüfblocks noch größer als ein Positionseintrag?
		if(pruefblocklaenge < sizeof(int))
		{
				//Wenn nicht, wird die Komprimierte Datei geschrieben und das Programm beendet.
			
				//Trennzeichen schreiben, um vom Datenblock zu trennen
			int delim = -2;
			output.write((char *)&delim, sizeof(int));
			output << data;
			//Jetzt noch die Ausgabedatei schließen, und das war's. Rücksprung!
			output.close();
			return;
			
		}
	}
}

void decompress()
{
		//Datei zum dekomprimieren öffnen
	cout << "Filename to decompress: ";
	string filename;
	cin >> filename;
	cin.ignore();
	
	fstream compressfile;
	compressfile.open(filename.c_str(), ios::in);
	
	string data = "";
	deque<int> matches;
	stack< deque<int> > fullmatches;
	
	
		//Kommandos lesen
	int c;
	while (compressfile.read((char *)&c, sizeof(int))) {
		if(c == -1)
		{
				//Trennzeichen zwischen Datenpaaren, also die Datenpaare kopieren...
			deque<int> tempmatches;
			while (!matches.empty()) {
				tempmatches.push_front(matches.back());
				matches.pop_back();
			}
				// ...und in die Datenpaarliste schieben
			fullmatches.push(tempmatches);
		} else if(c == -2) {
				//Jetzt kommen Daten, wir können aufhören mit Kommandolesen
			break;
		} else {
				//Kein Trennzeichen, also ist es ein Teil enes Datenpaares. Ab in die Liste damit!
			matches.push_front(c);
		}
	}
	
		//Bis hierhin waren es Kommandos.
	long offset = compressfile.tellg();
	
		//Dateiende herausfinden
	compressfile.seekg(0, ios::end);
	long datalength = compressfile.tellg();
	datalength++;
		//Dateilänge ist natürlich ohne die Kommandos zu betrachten, also wir die Kommandodatenlänge abgezogen...
	datalength -= offset;
		// ...und der Lesekopf auf das erste Datenbyte nach den Kommandos gesetzt.
	compressfile.seekg(offset);
	
		//Daten lesen und in den String schreiben
	char *tempdata = new char[datalength];
	compressfile.read(tempdata, datalength);
	data.append(tempdata);
	
		//Die Datei brauchen wir jetzt nicht mehr.
	compressfile.close();
	
		//Durch alle Datenpaare arbeiten
	while (!fullmatches.empty()) {
			//Ein Datenpaar aus dem Stapel holen
		matches = fullmatches.top();
		fullmatches.pop();
		
			//Blocklänge und Blockposition der vorhandenen Daten auslesen
		long blocklength = matches.front();
		matches.pop_front();
		long blockpos = matches.front();
		matches.pop_front();
		
			//Und jetzt für jede Kopieposition die passenden Daten schreiben
		while (!matches.empty()) {
				//Eine Position holen
			long postoinsert = matches.front();
			matches.pop_front();
			
				//Originaldaten auslesen
			string substring = data.substr(blockpos, blocklength);
				//Kopie schreiben
			data.insert(postoinsert, substring);
		}
	}
		//Ausgabedatei öffnen
	fstream output;
	output.open("decompressed.txt", ios::out);
	
		//Dekomprimierte Daten schreiben
	output << data;
		//Und die Datei wirder schließen
	output.close();
}


int main (int argc, const char * argv[])
{
	bool cont = true;
	while (cont) {
		cout << "DXCompress\n\nMenu:\n(1) Compress\n(2) Decompress\n(0) Quit\n\nPlease choose an option: ";
		int choise;
		cin >> choise;
		cin.ignore();
		if (choise == 0) {
			cont = false;
		} else if (choise == 1) {//Compress
			compress();
		} else if (choise == 2) {//Decompress
			decompress();
		} else {
			cont = false;
		}
		
	}
	
    return 0;
}

