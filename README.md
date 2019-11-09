# GeoChallenge AI bot

Youtube: https://www.youtube.com/watch?v=u6K2D9jS1ig

## Usage

- Go to Facebook and starts the game. Wait until it loads and shows the main menu.
- Select "English" language and set animation quality to "low" (the small icon on the left top corner). This is to make the game faster and smoother.
- Launch the bot.
- Set out the windows as desired. It is not mandatory to leave the bot windows visible, but it's advisable. Warning: the game window must NOT be ocludded by any other window.
- Press "Start", then "World tour". The bot will do the rest until finish one game.

At any time you can close the bot by pressing ESC or closing its window, but in that case the so far learned data will not be saved.

## About the directories

- `learndata`: Here it is saved all the bot learns. This directory is NOT included in the downloads, so the bot will fail a lot the first time you run it, then eventually it will learn and improve.
- `tessdata`: Files used by [tessract](http://code.google.com/p/tesseract-ocr/, the OCR engine.
- `data`: Images needed by the program.

For videos, the source code and more info: http://open-cv-bots.blogspot.com/

## Building

Use CMake. This program is designed for Windows only.
