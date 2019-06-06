#include "mem_hierarchy.h"

typedef struct decode_result {
  bool failed;
  char * message;
} decode_result;

decode_result decode(char * line, memory_hierarchy * hierarchy) {

  decode_result result;
  result.failed = false;
  result.message = "";

  char * cmd = strtok(line, " \n");
  if (cmd == NULL) {
    result.failed = true;
    result.message = "cannot parse line.";
    return result;
  }

  if (strcmp(cmd, "MR") == 0) {
    printf("[MR: %.2f]", get_miss_rate(hierarchy));
    return result;
  }
  if (strcmp(cmd, "FLUSH") == 0) {
    flush_cache(hierarchy);
    return result;
  }
  if (strcmp(cmd, "R") == 0){
    char * address_str = strtok(NULL, ", ");
    if(address_str == NULL) {
      result.failed = true;
      result.message = "missing address.";
      return result;
    }
    int address = atoi(address_str);
    if(address < 0 || address >= RAM_SIZE) {
      result.failed = true;
      result.message = "invalid address.";
      return result;
    }
    read_byte(hierarchy, address);
    return result;
  }
  if (strcmp(cmd, "W") == 0){
    char * address_str = strtok(NULL, ", ");
    if(address_str == NULL) {
      result.failed = true;
      result.message = "missing address.";
      return result;
    }
    int address = atoi(address_str);
    if(address < 0 || address >= RAM_SIZE) {
      result.failed = true;
      result.message = "invalid address.";
      return result;
    }
    char * value_str = strtok(NULL, ", ");
    if(address_str == NULL) {
      result.failed = true;
      result.message = "missing value.";
      return result;
    }
    unsigned char value = (unsigned char) atoi(value_str);
    write_byte(hierarchy, address, value);
    return result;
  }

  result.failed = true;
  result.message = "unrecognized command.";
  return result;
}


int main(int argc, char * argv[]) {

  if(argc == 1) {
    puts("Please, pass the name of the file to test as argument.");
    exit(1);
  }

  FILE * file;
  const char * filename = argv[1];

  file = fopen(filename, "r");
  if (file) {

    memory_hierarchy * hierarchy = init();
    //print_hierarchy(hierarchy, true, false);

    char * line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file) != -1){
      decode_result result = decode(line, hierarchy);
      if(result.failed){
        printf("[ERROR] Invalid line (%s) in file: %s\n", line, result.message);
      }else{
        printf("\n");
      };
    }

    print_hierarchy(hierarchy, true, false);
    destroy(hierarchy);

    fclose(file);
    if (line) free(line);
  } else {
    printf("Could't read file '%s'\n", filename);
  }

  return 0;
}
