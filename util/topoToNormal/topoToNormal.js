import Jimp from "jimp";

// Used to convert height maps into normal maps.
// Reference: https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
//            https://en.wikipedia.org/wiki/Sobel_operator

let inputFolderPath = "./input/";
let fileToConvert = "mars_topo.jpg";
let outputName = "mars_normal.png";
let filePath = inputFolderPath + fileToConvert;

function getHeightValue(i, j, img) {
  let pixel = img.getPixelColor(j, i);
  let { r, g, b, a } = Jimp.intToRGBA(pixel);
  let raw = b + (g << 8) + (r << 16);
  // Map from 0_FFFFFF to -1 to 1
  let height = (raw - 8388608.0) / 8388608.0;
  // console.log(height, raw, r, g, b);
  return height;
}

Jimp.read(filePath)
  .then((img) => {
    //First resize to lower res so that edges can be easier detected
    let resized = img.clone().resize(2048, Jimp.AUTO);
    let result = resized.clone();

    let strength = 1.2;

    for (let i = 0; i < resized.getHeight(); i++) {
      for (let j = 0; j < resized.getWidth(); j++) {
        let topIndex = i == 0 ? i : i - 1;
        let bottomIndex = i == resized.getHeight() - 1 ? i : i + 1;
        let leftIndex = j == 0 ? j : j - 1;
        let rightIndex = j == resized.getWidth() - 1 ? j : j + 1;

        let t = getHeightValue(topIndex, j, resized);
        let b = getHeightValue(bottomIndex, j, resized);
        let l = getHeightValue(i, leftIndex, resized);
        let r = getHeightValue(i, rightIndex, resized);
        let tl = getHeightValue(topIndex, leftIndex, resized);
        let tr = getHeightValue(topIndex, rightIndex, resized);
        let bl = getHeightValue(bottomIndex, leftIndex, resized);
        let br = getHeightValue(bottomIndex, rightIndex, resized);

        // Calculate sobel
        // X kernel : [-1, 0, 1], [-2, 0, 2], [-1, 0, 1]
        // Y kernel : [-1, -2, -1], [0, 0, 0], [1, 2, 1]
        let red = tl + 2 * l + bl - (tr + 2 * r + br);
        let green = tl + 2 * t + tr - (bl + 2 * b + br);
        let blue = 1.0 / strength;

        let maxMag = 4.1;

        if (Math.abs(red) > maxMag || Math.abs(green) > maxMag) {
          console.log(red, green, blue);
        }

        // Map from to 0-1
        red = (red + maxMag) / (2 * maxMag);
        green = (green + maxMag) / (2 * maxMag);

        // Normalize color
        let magnitude = Math.sqrt(red * red + green * green + blue * blue);
        red = Math.floor((red * 255) / magnitude);
        green = Math.floor((green * 255) / magnitude);
        blue = Math.floor((blue * 255) / magnitude);

        // console.log(red, green, blue);
        let color = Jimp.rgbaToInt(red, green, blue, 255);
        result.setPixelColor(color, j, i);
      }
    }
    result.write("../../assets/textures/" + outputName);
  })
  .catch((err) => console.log(err));
