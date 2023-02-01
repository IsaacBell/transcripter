#!/bin/bash

function print_directory_structure {
  local indent="$2  "
  local folder_name="$1"
  echo "$indent$folder_name"
  for item in "$folder_name"/*; do
    if [ -d "$item" ]; then
      print_directory_structure "$item" "$indent"
    elif [ -f "$item" ]; then
      echo "$indent  $(basename "$item")"
    fi
  done
}

print_directory_structure . ""
