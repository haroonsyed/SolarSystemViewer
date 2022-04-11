import Jimp from "jimp";

let fileToConvert = "./input/mercury_topo.jpg";

Jimp.read(fileToConvert)
  .then((img) => {
    let copy = img.clone();

    for (let i = 0; i < img.getHeight(); i++) {
      for (let j = 0; j < img.getWidth(); j++) {
        // Set normal based on surrounding pixels
        // https://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
        let up = copy.getPixelColor(i == 0 ? i : i - 1, j);
        let down = copy.getPixelColor(i == copy.getHeight() - 1 ? i : i + 1, j);
        let left = copy.getPixelColor(i, j == 0 ? j : j - 1);
        let right = copy.getPixelColor(i, j == copy.getWidth() - 1 ? j : j + 1);

        let red = ((left - right) / 2) * 255;
        let green = ((down - up) / 2) * 255;
        let blue = 255;
        red = math.abs((red * 2) / 255);
        let color = Jimp.rgbaToInt(red, green, blue, 255);
        copy.setPixelColor(color, i, j);
      }
    }

    copy.normalize();
    copy.write("./output/normal.jpg");
  })
  .catch((err) => console.log(err));
