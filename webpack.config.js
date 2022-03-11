const path = require('path');

const config = {
  target: 'web',
  mode: 'production',
  entry: './src/rom.mjs',
  context: path.resolve(__dirname, "."),
  module: {
    rules: [
      {
        test: /\.wasm$/,
        type: "javascript/auto",
        loader: "arraybuffer-loader"
      }
    ],
  },
  resolve: {
    extensions: ['.js'],
    fallback: {
      fs: false,
      crypto: false,
      path: false
    }
  },
  output: {
    path: path.resolve(__dirname, "dist"),
    filename: 'index.js',
    libraryExport: 'default',
    library: 'rom',
    libraryTarget: 'umd'
  }
}

module.exports = [config];
