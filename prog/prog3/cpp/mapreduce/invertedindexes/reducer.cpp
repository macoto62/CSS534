#include <iostream>
#include <string>
#include <sstream>
#include <map>

// Use map to manage if the filename has been populated already
std::map<std::string, int> docMap;
std::string keyword;

void emit( ) {
  // show content:
  std::string docList;
  for ( std::map<std::string,int>::iterator it = docMap.begin( ); it != docMap.end( ); ++it )
    docList += it->first + " " + std::to_string( it->second ) + " ";
  
  if ( !docList.empty( ) ) {
    std::cout << keyword << "\t" << docList << "\n";
  }
  docMap.clear( );
}

int main() {
    std::string line;
    std::string new_keyword;

    while( std::getline( std::cin, line ) ) {
      // YOUR WORK
      // Read tab-spearated "keyword\tfilename_count" lines from stdin
      // extract the keyword into new_keyword
      // if new_keyword is different from the previous keyword
      size_t tab_pos = line.find('\t');
      if(tab_pos ==std::string::npos) continue;
      new_keyword = line.substr(0, tab_pos);
      if(new_keyword  != keyword){
        emit();
        keyword = new_keyword;
      }
      
      // extract file name and count
      std::stringstream ss(line.substr(tab_pos+1));
      std::string filename;
      int count;
      if(!(ss >> filename >> count)) continue;
      // If hashmap doesn't contain this filename, put <filename, count>
      // as <key, value> into the hashmap
      // Otherwise, sum up a total and save as value into hashmap.
      docMap[filename]+=count;
    }

    emit( ); 

    return 0;
}
