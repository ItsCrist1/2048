# 2048
A pretty fast cross-platform C++ terminal implementation of the classic game [2048](https://en.m.wikipedia.org/wiki/2048_(video_game)).

# Features
- Saving and loading game states and custom game rules and board sizes
- Lots of settings for customization
- Custom colored cells
- Drawing tables with ASCII table symbols

# Compatibility
Compatibility with any UNIX system (any linux distro, Android, MacOS etc) is almost 100% guaranteed.
Compatibility with Windows (7+) is still questionable since Windows doesn't appear to natively support RGB ANSI color codes, so you will want to use the --nocolor argument.

# Building
Requirements: git (or gh) and cmake
```
git clone https://www.github.com/ItsCrist1/2048.git
cd 2048/bin
cmake ..
cd ..
./exec.sh
```

# Running
You can run [this](https://github.com/ItsCrist1/2048/blob/main/bin/exec) executable
```
git clone https://www.github.com/ItsCrist1/2048.git
cd 2048
./bin/exec
```

# Contact
You can contact me at cristi9270@gmail.com or cristi123612 on discord.

# License
This project is licensed under [The MIT License](https://github.com/ItsCrist1/2048/blob/main/LICENSE.txt).
