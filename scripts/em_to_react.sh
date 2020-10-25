# help usage
print_usage() {
  printf "TODO"
}

# default
EM_FOLDER=c++/build/em
REACT_FOLDER="react-emcc-gl"


# process args
while getopts 'e:r:' flag; do
  case "${flag}" in
    e) EM_FOLDER="${OPTARG}" ;;
    r) REACT_FOLDER="${OPTARG}" ;;
    *) print_usage
       exit 1 ;;
  esac
done
shift "$((OPTIND - 1))"

JS_FILE=$EM_FOLDER/$1.js
if [ ! -f "$JS_FILE" ]; then
    echo "$FILE does not exist."
    exit
fi

DATA_FILE=$EM_FOLDER/$1.data
WASM_FILE=$EM_FOLDER/$1.wasm

mkdir -p $REACT_FOLDER/public/generated
mkdir -p $REACT_FOLDER/src/generated

cp $DATA_FILE $REACT_FOLDER/public/generated/$1.data
cp $WASM_FILE $REACT_FOLDER/public/generated/$1.wasm

MOD_JS_FILE=$REACT_FOLDER/src/generated/$1.js
cp $JS_FILE $MOD_JS_FILE

sed -i '1s;^;\/* eslint-disable *\/;' ${MOD_JS_FILE}
sed -i "s|wasmBinaryFile=\"$1.wasm\"|wasmBinaryFile=\`\${process.env.PUBLIC_URL}/generated/$1.wasm\`|" ${MOD_JS_FILE}
sed -i "s|REMOTE_PACKAGE_BASE=\"$1.data\"|REMOTE_PACKAGE_BASE=\"generated/$1.data\"|" ${MOD_JS_FILE}
sed -i "s|wasmBinaryFile=locateFile(wasmBinaryFile)|\/*wasmBinaryFile=locateFile(wasmBinaryFile)*\/|" ${MOD_JS_FILE}
