import Jimp from "jimp";


// Used to convert height maps into normal maps.
// Reference: https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
//            https://en.wikipedia.org/wiki/Sobel_operator

let inputFolderPath = "./input/"
let fileToConvert = "mercury_topo.png";
let filePath = inputFolderPath + fileToConvert;

function getHeightValue(i,j,img) {
    let pixel = img.getPixelColor(j,i);
    let {r, g, b, a} = Jimp.intToRGBA(pixel);
    let raw = b + (g<<8) + (r<<16);
    // Map from 0_FFFFFF to -1 to 1
    let height = (raw-8388608.0)/8388608.0;
    // console.log(height, raw, r, g, b);
    return height;
}

Jimp.read(filePath)
  .then((img) => {
    let result = img.clone();
    let strength = 10.0;

    for (let i = 0; i < result.getHeight(); i++) {
      for (let j = 0; j < result.getWidth(); j++) {

        let topIndex = i == 0 ? i : i - 1
        let bottomIndex = i == img.getHeight() - 1 ? i : i + 1;
        let leftIndex = j == 0 ? j : j - 1;
        let rightIndex = j == img.getWidth() - 1 ? j : j + 1;

        let t = getHeightValue(topIndex, j, img);
        let b = getHeightValue(bottomIndex, j, img);
        let l = getHeightValue(i, leftIndex, img);
        let r = getHeightValue(i, rightIndex, img);
        let tl = getHeightValue(topIndex, leftIndex, img);
        let tr = getHeightValue(topIndex, rightIndex, img);
        let bl = getHeightValue(bottomIndex, leftIndex, img);
        let br = getHeightValue(bottomIndex, rightIndex, img);

        // Calculate sobel 
        // X kernel : [-1, 0, 1], [-2, 0, 2], [-1, 0, 1]
        // Y kernel : [-1, -2, -1], [0, 0, 0], [1, 2, 1]
        let red = (tr + 2*r + br) - (tl + 2*l + bl);
        let green = (bl+2*b+br) - (tl+2*t+tr);

        let maxMag = 4.1;

        if(Math.abs(red) > maxMag || Math.abs(green) > maxMag) {
          console.log(red, green, blue)
        }

        // Map from to 0-1
        red = ((red+maxMag)/(2*maxMag));
        green = ((green+maxMag)/(2*maxMag));
        let blue = 1.0/strength;


        // Normalize color
        let magnitude = Math.sqrt(red*red + green*green + blue*blue);
        red = Math.floor(red*255/magnitude);
        green = Math.floor(green*255/magnitude);
        blue = Math.floor(blue*255/magnitude);
        
        // console.log(red, green, blue);
        let color = Jimp.rgbaToInt(red, green, blue, 255);
        result.setPixelColor(color, j, i);
    

      }
    }
    // Keep maps at 8k res
    result.resize(8192,Jimp.AUTO);
    result.write('./output/' + fileToConvert + "_normal.png");

  })
  .catch((err) => console.log(err));


