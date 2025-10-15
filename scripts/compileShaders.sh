#!/bin/bash

# Create build directory if it doesn't exist
mkdir -p ./build

# Find all shader files in the shaders directory
for shader in ./shaders/*; do
    # Get the filename without path
    filename=$(basename "$shader")
    
    # Get the extension
    extension="${filename##*.}"
    
    # Get the name without extension
    name="${filename%.*}"
    
    # Determine output name based on shader type
    case "$extension" in
        vert)
            output="${name}Vert.spv"
            ;;
        frag)
            output="${name}Frag.spv"
            ;;
        comp)
            output="${name}Comp.spv"
            ;;
        geom)
            output="${name}Geom.spv"
            ;;
        tesc)
            output="${name}Tesc.spv"
            ;;
        tese)
            output="${name}Tese.spv"
            ;;
        *)
            echo "Unknown shader type: $extension for file $filename"
            continue
            ;;
    esac
    
    # Compile the shader
    echo "Compiling $filename -> $output"
    glslc "$shader" -o "./build/$output"
    
    # Check if compilation was successful
    if [ $? -eq 0 ]; then
        echo "  ✓ Success"
    else
        echo "  ✗ Failed to compile $filename"
        exit 1
    fi
done

echo ""
echo "All shaders compiled successfully!"