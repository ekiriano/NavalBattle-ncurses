#ifndef __WINDOW_H
#define __WINDOW_H

extern "C" {
#include <curses.h>
}
#include <string>

// Ensemble de couleurs possibles (fond+texte)
enum Color {
  WBLACK,  // couleur fond = noir ,   couleur texte = blanc
  WCYAN,   // couleur fond = cyan,    couleur texte = blanc
  WBLUE,   // couleur fond = bleu,    couleur texte = blanc
  WYELLOW, // couleur fond = jaune,   couleur texte = blanc
  WGREEN,  // couleur fond = vert,    couleur texte = blanc
  WMAGENTA,// couleur fond = magenta, couleur texte = blanc
  WRED,	   // couleur fond = rouge,   couleur texte = blanc
  BWHITE,  // couleur fond = blanc,   couleur texte = blanc
  BCYAN,   // couleur fond = cyan,    couleur texte = noir
  BBLUE,   // couleur fond = bleu,    couleur texte = noir
  BYELLOW, // couleur fond = jaune,   couleur texte = noir
  BGREEN,  // couleur fond = vert,    couleur texte = noir 
  BMAGENTA,// couleur fond = magenta, couleur texte = noir
  BRED,    // couleur fond = rouge,   couleur texte = noir
};



// fonction pour demarrer le mode console graphique
void startProgramX();
// fonction pour arreter le mode console graphique
void stopProgramX();


class Window {
 private:
  int height,width,startx,starty;
  WINDOW* win, *frame;
  Color colorwin, colorframe;
  char bord;
  void update() const;

 public:

  // constructeur d'un fenetre de hauteur=h, largeur=w dont le coin superieur gauche
  // a pour coordonnée (x,y), le caractère c est utilisé pour définir la bordure
  Window(int h,int w, int x, int y, char c='+');

  ~Window();

  // fonction permettant d'afficher une variable s de type (string ou char)
  // à la position (x,y) dans la fenetre.
  // si un couleur est spécifié l'affichage utilise cette couleur, sinon la couleur de la fenêtre est utilisée
  void print(int x, int y, std::string s, Color c) const;
  void print(int x, int y, char s, Color c) const;
  void print(int x, int y, std::string s) const;
  void print(int x, int y, char s) const; 

  
  // accesseurs
  int getX() const;        // récupère l'abscisse du coin supérieur gauche de la fenêtre 
  int getY() const;        // récupère l'ordonnée du coin supérieur gauche de la fenêtre 
  int getHauteur() const ; // récupère la hauteur de la fenêtre
  int getLargeur() const ; // récupère la largeur de la fenêtre

  Color getCouleurBordure() const; // récupère la couleur de la bordure
  Color getCouleurFenetre() const; // récupère la couleur de la fenêtre
  void setCouleurBordure(Color);   // modifie la couleur de la bordure
  void setCouleurFenetre(Color);   // modifie la couleur de la fenêtre (ATTENTION, tout le contenu de la fenêtre prend la couleur)

  void clear() const; // enleve tout le contenu de la fenêtre

};











#endif
