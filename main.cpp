#include <fstream>
#include <iostream>
#include <memory>

#include "lexer.h"
#include "parser.h"

using namespace std;
using namespace cplr;

int main(int argc, char *argv[]) {
	if (argc == 1) {
		cout << "The file to be compiled wasn\'t attached." << endl;
	} else {
		ifstream prog(argv[1]);
		if (!prog) {
			cout << "Can\'t open file." << endl;
		} else {
			prog.seekg(0, ios::end);
			size_t size = prog.tellg();
			string code(size, '\0');
			prog.seekg(0);
			prog.read(&code[0], size);
			vector<shared_ptr<node>> tokens = tokenize(code.begin(), code.end());
			parser prsr;
			prsr.parse(tokens.begin(), tokens.end());
			prsr.run();
		}
	}

	return 0;
}
