all:
	g++ game.cpp window.cpp map.cpp menu.cpp player.cpp ship.cpp frameCell.cpp move.cpp score.cpp -o game -lncurses
clean:
	rm -f hello

