#include <cmath>
#include <cstdlib>
#include <iostream>
#include <allegro.h>
#include <winalleg.h>   //zamiast windows.h, ktore gryzie sie z allegro

using namespace std;

#define CZARNY makecol(0,0,0)

//Bitmapy uzywane w programie
BITMAP *bmp;
BITMAP *obraz=NULL;
BITMAP *obraz_wynikowy;
BITMAP *przycisk_otworz;
BITMAP *przycisk_zapisz;
BITMAP *przycisk_progowanie;
BITMAP *przycisk_zmiana_jasnosci;
BITMAP *przycisk_korekcja_gamma;
BITMAP *przycisk_pomoc;
BITMAP *wybor_progu;
BITMAP *wybor_jasnosci;
BITMAP *wybor_gamma;

PALETTE pal;    //paleta, do wczytywania obrazow

int wczytano_obraz=0;   //zmienna logiczna
int glebia_kolorow=32;
int czas;               //zeby zapewnic pewnien przedzial czasu
int przedzial_czasu=30; //na wcisniecie przycisku
int pozycja_x_przyciskow=820;
int x1=0,x2=0,y1_z=0,y2=0;  //wspolrzedne elipsy
int r=240,g=0,b=0;
int a_elipsy,b_elipsy;  //a - pozioma przekatna elipsy, b- pionowa
int x_elipsy, y_elipsy; //wspolrzedne srodka elipsy
int lut[256];   //tabela LUT
int liczba_stron_pomocy=2;

//zmienne logiczne, zeby uproscic sprawdzanie, czy wcisnieto przycisk
bool kliknieto_przycisk_otworz;
bool kliknieto_przycisk_progowanie;
bool kliknieto_przycisk_zmiana_jasnosci;
bool kliknieto_przycisk_korekcja_gamma;
bool kliknieto_przycisk_zapisz;
bool kliknieto_przycisk_pomoc;
bool wcisniety_lpm;
bool zaznaczono=false;

char nazwa_pliku[ MAX_PATH ] = "";  //nazwa pliku do odczytu/zapisu

///////prototypy funkcji
int inicjalizacja();
int zakoncz();
void otworz();
void zapisz();
void klik();
void zaznaczenie();
void rysuj_zaznaczenie();
void progowanie();
void zmien_jasnosc();
void korekcja_gamma();
void przyciski();
int pomoc();
int sqr(int liczba);
int main(int arg, char **argv);


int sqr(int liczba) { //potega 2 stopnia
    return liczba*liczba;
}

void inicjuj_lut() {    //inicjalizacja rablicy LUT do wartości standardowych
    for (int i=0;i<256;i++) {
        lut[i]=i;
    }
}

//-------------------Inicjalizacja-----------------------
int inicjalizacja() {   //inicjalizacja biblioteki allegro i zmiennych
    if(allegro_init()!=0){
        allegro_message("Wystapil blad przy inicjalizacji biblioteki.\n%s", allegro_error);
        return 1;
    }
    install_keyboard();
    install_timer();
    install_mouse();
    set_color_depth(glebia_kolorow);
    set_palette(default_palette);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, 1000, 600, 0, 0);//800x600 na obraz, dodakowe 200 pikseli na przyciski

    bmp=create_bitmap(screen->w,screen->h);//tworzenie bufora na ktory wklejane sa wszystkie bitmapy i tekst
    obraz=create_bitmap(800,600);
    obraz_wynikowy=create_bitmap(800,600);
    przycisk_otworz=create_bitmap(160,40);
    przycisk_zapisz=create_bitmap(160,40);
    przycisk_progowanie=create_bitmap(160,40);
    przycisk_zmiana_jasnosci=create_bitmap(160,40);
    przycisk_korekcja_gamma=create_bitmap(160,40);
    przycisk_pomoc=create_bitmap(160,40);
    wybor_progu=create_bitmap(265,50);
    wybor_jasnosci=create_bitmap(265,50);
    wybor_gamma=create_bitmap(265,50);

    inicjuj_lut();

    return 0;
}

//------------------Zakonczenie programu---------------------
int zakoncz() { //usuwanie bitmap i konczenie pracy biblioteki allegro
    destroy_bitmap(bmp);
    destroy_bitmap(obraz);
    destroy_bitmap(przycisk_otworz);
    destroy_bitmap(przycisk_zapisz);
    destroy_bitmap(przycisk_progowanie);
    destroy_bitmap(przycisk_zmiana_jasnosci);
    destroy_bitmap(przycisk_korekcja_gamma);
    destroy_bitmap(przycisk_pomoc);
    destroy_bitmap(wybor_jasnosci);
    destroy_bitmap(wybor_progu);
    destroy_bitmap(wybor_gamma);
    allegro_exit();
    return 0;
}

//-----------reagowanie na klikniecie mysza-------------------
void klik() {
    int mx=mouse_x;//pozycja x myszy
    int my=mouse_y;//pozycja y myszy

    if (mouse_b==1) {//jesli wcisnieto LPM
        if (mx>=pozycja_x_przyciskow && mx<=pozycja_x_przyciskow+przycisk_otworz->w && my>=20 && my<=60)
            kliknieto_przycisk_otworz=true;
        if (mx>=pozycja_x_przyciskow && mx<=pozycja_x_przyciskow+przycisk_zapisz->w && my>=80 && my<=120)
            kliknieto_przycisk_zapisz=true;
        if (mx>=pozycja_x_przyciskow && mx<=pozycja_x_przyciskow+przycisk_progowanie->w && my>=400 && my<=440)
            kliknieto_przycisk_progowanie=true;
        if (mx>=pozycja_x_przyciskow && mx<=pozycja_x_przyciskow+przycisk_zmiana_jasnosci->w && my>=460 && my<=500)
            kliknieto_przycisk_zmiana_jasnosci=true;
        if (mx>=pozycja_x_przyciskow && mx<=pozycja_x_przyciskow+przycisk_korekcja_gamma->w && my>=520 && my<=560)
            kliknieto_przycisk_korekcja_gamma=true;
        if (mx>=pozycja_x_przyciskow && mx<=pozycja_x_przyciskow+przycisk_pomoc->w && my>=200 && my<=240)
            kliknieto_przycisk_pomoc=true;
        mouse_b=0;
    }
}

//----------------zaznaczenie------------------------
void zaznaczenie(int przycisk) {
    if (x1 == 0 && wcisniety_lpm == false && mouse_x <800) { //jezeli nie ma zaznaczenia, to trzeba je stworzyc
        //zaznaczono=true;
        wcisniety_lpm=true;
        x1=mouse_x;
        y1_z=mouse_y;
    }

    else if (x1 != 0 && wcisniety_lpm==false && mouse_x <800){//jezeli jest juz zaznaczenie i mysz znajduje sie nad obrazem
        wcisniety_lpm=true;
        zaznaczono=true;
        x2=mouse_x;
        y2=mouse_y;
        if (x2>800) x2=800;
        if (y2>600) y2=600;
        x_elipsy=(x1+x2)/2;     //ustawianie srodka
        y_elipsy=(y1_z+y2)/2;   //elipsy
        a_elipsy=abs(x2-x_elipsy);//a - "promien poziomy"
        b_elipsy=abs(y2-y_elipsy);//b - "promien pionowy"
                //ellipsefill(bmp,x_elipsy,y_elipsy,a_elipsy,b_elipsy,makecol(255,0,0));
    }

    if (przycisk == 2) {    //2 = prawy przycisk myszy, usuwanie zaznaczenia
        x1=0;
        y1_z=0;
        x2=0;
        y2=0;
        a_elipsy=0;
        b_elipsy=0;
        zaznaczono=false;
    }
}

//------------rysowanie zaznaczenia-------------------------
void rysuj_zaznaczenie() {
    if (r>=255) r=100;
    else r+=2;
    if (zaznaczono) {
        rect(bmp,x1,y1_z,x2,y2,makecol(r/2,r/2,r/2));   //prostokat opisany na elipsie
        ellipse(bmp,(x1+x2)/2,(y1_z+y2)/2,abs((x2-x1)/2),abs((y2-y1_z)/2),makecol(b,g,r));
    }
}

//--------------otwarcie pliku---------------------------
void otworz() {
        //if (obraz) destroy_bitmap(obraz);

        //WinApi:

        OPENFILENAME otworz_plik;

        //zerowanie struktury:
        ZeroMemory( & otworz_plik, sizeof( otworz_plik ) );
        otworz_plik.lStructSize = sizeof( otworz_plik );

        otworz_plik.lpstrFilter = "Bitmapy (*.bmp)\0*.bmp\0Wszystkie pliki\0*.*\0";//typ plikow, ktory jest wyswietlany

        otworz_plik.nMaxFile = MAX_PATH;    //to, w razie gdyby sciezka byla za dluga
        otworz_plik.lpstrFile = nazwa_pliku;
        otworz_plik.lpstrDefExt = "bmp";    //domysne rozszerzenie = .bmp
        otworz_plik.Flags = OFN_FILEMUSTEXIST;//flaga - plik musi istniec

        if (GetOpenFileName( & otworz_plik )) { //jesli uztkownik otworzyl jakiś plik

        set_color_conversion(COLORCONV_32_TO_8);
        obraz=load_bitmap(nazwa_pliku,pal); //zaladowanie obrazu
        select_palette(pal);
        if (!obraz) {
                allegro_message("Nie odnaleziono pliku: %s!\nSprawdz nazwe pliku i podana sciezke",nazwa_pliku);
                wczytano_obraz=0;
        }
        else {
                blit(obraz,bmp,0,0,0,0,800,600);
                wczytano_obraz=1;
                //unselect_palette();
        }
        }

}

//--------------Zapisywanie pliku na dysku-----------------
void zapisz() {
            //WinApi:
            OPENFILENAME zapisz_plik;

            //zerowanie struktury:
            ZeroMemory( & zapisz_plik, sizeof( zapisz_plik ) );
            zapisz_plik.lStructSize = sizeof( zapisz_plik );

            zapisz_plik.lpstrFilter = "Bitmapy (*.bmp)\0*.bmp\0Wszystkie pliki\0*.*\0";//typ plikow, ktory jest wyswietlany
            strcpy(nazwa_pliku,"obraz_wynikowy.bmp");   //nazwa, pod jaka bedzie zapisany plik
            zapisz_plik.nMaxFile = MAX_PATH;    //gdyby sciezka byla zbyt dluga
            zapisz_plik.lpstrFile = nazwa_pliku;//nazwa
            zapisz_plik.lpstrDefExt = "bmp";    //domyslne rozszerzenie
            zapisz_plik.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;//flagi - plik musi istniec, jezeli istnieje,
                                                                        //to trzeba potwierdzic nadpisanie

            set_color_conversion(COLORCONV_32_TO_8);

            if (GetSaveFileName( & zapisz_plik )) { //jesli uzytkownik zapisze plik
                int kod_bledu=save_bitmap(nazwa_pliku, obraz, pal);
                if (!kod_bledu) allegro_message("zapisano jako %s",nazwa_pliku);
                else allegro_message("Zapis nie powiodl sie!");
            }
}

//----------------progowanie--------------------
void progowanie() {
    int prog=127;   //domyslny prog - polowa zakresu (0-255)

    while(!key[KEY_ENTER] && mouse_b!=1) {  //dopoki uzytkownik nie potwierdzi wartosci
    int mx=mouse_x;
    //wyswietlenie okna z wyborem wartosci
    rectfill(wybor_progu,0,0,wybor_progu->w,wybor_progu->h,makecol(100,100,100));
    textout_ex(wybor_progu,font,"Wybierz prog za pomoca myszy",0,5,makecol(0,0,0),-1);
    textout_ex(wybor_progu,font,"i nacisnij ENTER lub LPM",0,15,makecol(0,0,0),-1);
    rectfill(wybor_progu,0,25,265,35,makecol(80,80,80));
    rectfill(wybor_progu,prog,25,prog+10,35,makecol(150,150,150));
    textprintf_ex(wybor_progu, font,wybor_progu->w/2,40, makecol(0,0,0), - 1, "%d",prog);

        if (/*mouse_b==1 && */mx>=screen->w/3 && mx<=screen->w/3+255) {//wybieranie wartosci za pomoca myszy
            prog=mouse_x-screen->w/3;
        }
        if (wczytano_obraz) blit(obraz,bmp,0,0,0,0,800,600);
        //rest(10);

        //rysowanie tablicy LUT
        rectfill(bmp,(screen->w/3),30,(screen->w/3)+255,30+255,makecol(255,255,255));

        for (int i=0;i<256;i++) {   //ustawianie wartosci poszczegolnych elementow tablicy LUT
        if (i > prog)
            lut[i]=255;
        else if (i < prog)
            lut[i]=0;
        //else lut[i]=jasnosc_pom+i;

        putpixel(bmp,(screen->w/3) + i, 255 + 30 + i*-1,makecol(100,100,100));
        putpixel(bmp,(screen->w/3) + i, 255 + 30 + lut[i]*-1,makecol(255,0,0));
        putpixel(bmp,(screen->w/3) + i, 255 + 31 + lut[i]*-1,makecol(255,0,0));
        if (i==prog || i==prog+1) line(bmp,(screen->w/3) + i, 30,(screen->w/3) + i,255+30,makecol(255,0,0));
        }


        blit(wybor_progu,bmp,0,0,screen->w/3,screen->h/2,screen->w,screen->h);
        blit(bmp,screen,0,0,0,0,screen->w,screen->h);
    }


   // if (wczytano_obraz) blit(obraz,bmp,0,0,0,0,800,600);
   // blit(wybor_jasnosci,bmp,0,0,screen->w/3,screen->h/2,screen->w,screen->h);
    blit(bmp,screen,0,0,0,0,screen->w,screen->h);

     int glebia = bitmap_color_depth(obraz);    //zeby bylo wiadomo, czy wczytujemy plik 8 bitowy czy inny
    int j,k;
    for (k=0;k<obraz->w;k++)    //k - wspolrzedna x
    for (j=0;j<obraz->h;j++) {  //j - wspolrzedna y
    if (zaznaczono) {   //ograniczenie progowania do wnetrza elipsy
            float x_float=(float(k)-float(x_elipsy))*(float(k)-float(x_elipsy));    //x^2
            float a_float=float(a_elipsy)*float(a_elipsy);                          //a^2
            float y_float=(float(j)-float(y_elipsy))*(float(j)-float(y_elipsy));    //y^2
            float b_float=float(b_elipsy)*float(b_elipsy);                          //b^2

    //x^2/a^2 + y^2/b^2 = 1 - rownanie elipsy
    if ((x_float/a_float)+(y_float/b_float) <= 1.0) {   //jesli piksel miesci sie w elipsie
    if (glebia == 8) {
        if (getpixel(obraz,k,j) < prog)
            putpixel(obraz,k,j,makecol(0,0,0));
        else putpixel(obraz,k,j,makecol(255,255,255));
    }
    else {//jezeli glebia 24 bit
        if (getpixel(obraz,k,j)<makecol(prog,prog,prog))
            putpixel(obraz,k,j,makecol(0,0,0));
        else putpixel(obraz,k,j,makecol(255,255,255));
    }
    }
    }
    else { //jezeli nie ma zaznaczenia
        if (glebia == 8) {
            if (getpixel(obraz,k,j)< prog)
            putpixel(obraz,k,j,makecol(0,0,0));
            else putpixel(obraz,k,j,makecol(255,255,255));
        }
        else {//glebia 24 bit
            if (getpixel(obraz,k,j)<makecol(prog,prog,prog))
            putpixel(obraz,k,j,makecol(0,0,0));
            else putpixel(obraz,k,j,makecol(255,255,255));
        }
    }
    }
}

//-----------------zmiana jasnosci obrazu------------
void zmiana_jasnosci() {
    int jasnosc=127;
    inicjuj_lut();

    while(!key[KEY_ENTER] && mouse_b!=1) {  //dopoki uzytkownik nie potwierdzi wartosci
    int mx=mouse_x;
    //wyswietlenie okna z wyborem wartosci
    rectfill(wybor_jasnosci,0,0,wybor_jasnosci->w,wybor_jasnosci->h,makecol(100,100,100));
    textout_ex(wybor_jasnosci,font,"Wybierz wartosc pojasnienia za pomoca myszy",0,5,makecol(0,0,0),-1);
    textout_ex(wybor_jasnosci,font,"i nacisnij ENTER lub LPM",0,15,makecol(0,0,0),-1);
    rectfill(wybor_jasnosci,0,25,265,35,makecol(80,80,80));
    rectfill(wybor_jasnosci,jasnosc,25,jasnosc+10,35,makecol(150,150,150));
    textprintf_ex(wybor_jasnosci, font,wybor_jasnosci->w/2,40, makecol(0,0,0), - 1, "%d",jasnosc-127);

        if (/*mouse_b==1 && */mx>=screen->w/3 && mx<=screen->w/3+255) {//wybor wartosci za pomoca myszy
            jasnosc=mouse_x-screen->w/3;
        }
        //rectfill(wybor_progu,prog+5,15,prog+15,25,makecol(150,150,150));
        //rest(10);
        if (wczytano_obraz) blit(obraz,bmp,0,0,0,0,800,600);
        blit(wybor_jasnosci,bmp,0,0,screen->w/3,screen->h/2,screen->w,screen->h);
        //blit(bmp,screen,0,0,0,0,screen->w,screen->h);
    //}

    int jasnosc_pom=jasnosc-127;

    //rysowanie tablicy LUT
    rectfill(bmp,(screen->w/3),30,(screen->w/3)+255,30+255,makecol(255,255,255));

    for (int i=0;i<256;i++) {   //ustawianie wartosci poszczegolnych elementow tablicy LUT
        if (jasnosc_pom+i >255 )
            lut[i]=255;
        else if (jasnosc_pom+i <0)
            lut[i]=0;
        else lut[i]=jasnosc_pom+i;

        putpixel(bmp,(screen->w/3) + i, 255 + 30 + i*-1,makecol(100,100,100));
        putpixel(bmp,(screen->w/3) + i, 255 + 30 + lut[i]*-1,makecol(255,0,0));
        putpixel(bmp,(screen->w/3) + i, 255 + 31 + lut[i]*-1,makecol(255,0,0));
        if (i == 0 || i == 255) line(bmp,(screen->w/3) +i, 255 +30 + i*-1,(screen->w/3) + i,255 + 30 + lut[i]*-1,makecol(255,0,0));
    }

   // if (wczytano_obraz) blit(obraz,bmp,0,0,0,0,800,600);
   // blit(wybor_jasnosci,bmp,0,0,screen->w/3,screen->h/2,screen->w,screen->h);
    blit(bmp,screen,0,0,0,0,screen->w,screen->h);

    }

    int kolor,r,g,b;
    int j,k;
    for (k=0;k<obraz->w;k++) //k - wspolrzedna x
    for (j=0;j<obraz->h;j++) {//j - wspolrzedna y
        kolor=getpixel(obraz,k,j);
        //okreslenie kazdej skladowej koloru oddzielnie na podstawie tablicy LUT
        r=getr(kolor);
        g=getg(kolor);
        b=getb(kolor);

        r=lut[r];
        g=lut[g];
        b=lut[b];

        kolor=makecol(r,g,b);

        if (zaznaczono) {
            float x_float=(float(k)-float(x_elipsy))*(float(k)-float(x_elipsy));    //x^2
            float a_float=float(a_elipsy)*float(a_elipsy);                          //a^2
            float y_float=(float(j)-float(y_elipsy))*(float(j)-float(y_elipsy));    //y^2
            float b_float=float(b_elipsy)*float(b_elipsy);                          //b^2

        if ((x_float/a_float)+(y_float/b_float) <= 1.0) {//jesli punkt miesci sie w elipsie
            putpixel(obraz,k,j,kolor);
        }
    }
    else putpixel(obraz,k,j,kolor);//jesli nie zaznaczono
    }
}

//-----------------zmiana wspolczynnika gamma------------
void korekcja_gamma() {
    int gamma=127;
    inicjuj_lut();

    while(!key[KEY_ENTER] && mouse_b!=1) {//dopoki uzytkownik nie potwierdzi wartosci
    int mx=mouse_x;
    //wyswietlenie okna z wyborem wartosci
    rectfill(wybor_gamma,0,0,wybor_gamma->w,wybor_gamma->h,makecol(100,100,100));
    textout_ex(wybor_gamma,font,"Wybierz wartosc gamma za pomoca myszy",0,5,makecol(0,0,0),-1);
    textout_ex(wybor_gamma,font,"i nacisnij ENTER lub LPM",0,15,makecol(0,0,0),-1);
    rectfill(wybor_gamma,0,25,265,35,makecol(80,80,80));
    rectfill(wybor_gamma,gamma,25,gamma+10,35,makecol(150,150,150));

        //zamian wartosci (0-255) na liczbe zmiennoprzecinkowa
        float f_gamma;
        int pom_gamma=gamma-127;

        if (pom_gamma<=0)
            f_gamma=float(pom_gamma)/127 + 1;
        else if (pom_gamma >= 12)f_gamma=(float(pom_gamma)/12.0);
        else f_gamma=1+(float(pom_gamma)/127.0);

        if (f_gamma<0.1) f_gamma=0.1;   //minimalna wartosc = 0.1
        if (f_gamma>10) f_gamma=10;     //maksymalna wartosc = 10

    textprintf_ex(wybor_gamma, font,wybor_gamma->w/2,40, makecol(0,0,0), - 1, "%.2f",f_gamma);  //wysiwetlenie wartosci

        if (/*mouse_b==1 && */mx>=screen->w/3 && mx<=screen->w/3+255) {//wybieranie wartosci za pomoca myszy
            gamma=mouse_x-screen->w/3;
        }
        //rectfill(wybor_progu,prog+5,15,prog+15,25,makecol(150,150,150));
        if (wczytano_obraz) blit(obraz,bmp,0,0,0,0,800,600);
        blit(wybor_gamma,bmp,0,0,screen->w/3,screen->h/2,screen->w,screen->h);
        //blit(bmp,screen,0,0,0,0,screen->w,screen->h);


    //rysowanie tablicy LUT
    rectfill(bmp,(screen->w/3),30,(screen->w/3)+255,30+255,makecol(255,255,255));

    for (int i=0;i<256;i++) {   //ustawianie wartosci w tablicy LUT
        if ((255 * pow(i/255.0, 1/f_gamma)) > 255)
            lut[i]=255;
        else lut[i]=255 * pow(i/255.0, 1/f_gamma);

        putpixel(bmp,(screen->w/3) + i, 255 + 30 + i*-1,makecol(100,100,100));
        //putpixel(bmp,(screen->w/3) + i, 255 + 30 + lut[i]*-1,makecol(255,0,0));
        //putpixel(bmp,(screen->w/3) + i, 255 + 31 + lut[i]*-1,makecol(255,0,0));
        if (i<255) {    //rysowanie linii 2 razy, zeby byla elpiej widoczna
                line(bmp,(screen->w/3) + i, 255 + 30 + lut[i]*-1,(screen->w/3) + i+1,255 + 30 + lut[i+1]*-1,makecol(255,0,0));
                line(bmp,(screen->w/3) + i, 255 + 31 + lut[i]*-1,(screen->w/3) + i+1,255 + 31 + lut[i+1]*-1,makecol(255,0,0));
        }
    }

    blit(bmp,screen,0,0,0,0,screen->w,screen->h);

    }
    int kolor,r,g,b;
    int j,k;
    for (k=0;k<obraz->w;k++)//k- wspolrzedna x
    for (j=0;j<obraz->h;j++) {//j - wspolrzedna y
        //okreslenie kazdej skladowej koloru oddzielnie na podstawie tablicy LUT
        kolor=getpixel(obraz,k,j);
        r=getr(kolor);
        g=getg(kolor);
        b=getb(kolor);

        r=lut[r];
        g=lut[g];
        b=lut[b];

        kolor=makecol(r,g,b);

        if (zaznaczono) {
            float x_float=(float(k)-float(x_elipsy))*(float(k)-float(x_elipsy));    //x^2
            float a_float=float(a_elipsy)*float(a_elipsy);                          //a^2
            float y_float=(float(j)-float(y_elipsy))*(float(j)-float(y_elipsy));    //y^2
            float b_float=float(b_elipsy)*float(b_elipsy);                          //b^2

        if ((x_float/a_float)+(y_float/b_float) <= 1.0) {//jesli piksel miesci sie w elipsie
            putpixel(obraz,k,j,kolor);
        }
    }
    else putpixel(obraz,k,j,kolor);//gdy nie miesci sie w elipsie
    }
}

//--------------Rysowanie przyciskow-------------------
void przyciski() {
        rectfill(przycisk_otworz,0,0,160,40,makecol(120,120,120));//przycisk otworz plik
        textout_ex(przycisk_otworz,font,"Otworz plik",0,przycisk_otworz->h/2,CZARNY,-1);
        blit(przycisk_otworz,bmp,0,0,pozycja_x_przyciskow,20,980,40);

        rectfill(przycisk_zapisz,0,0,160,40,makecol(120,120,120)); //przycisk zapisz plik
        if (wczytano_obraz) textout_ex(przycisk_zapisz,font,"Zapisz plik",0,przycisk_zapisz->h/2,CZARNY,-1);
        else textout_ex(przycisk_zapisz,font,"Zapisz plik",0,przycisk_zapisz->h/2,makecol(150,150,150),-1);
        blit(przycisk_zapisz,bmp,0,0,pozycja_x_przyciskow,80,980,40);

        rectfill(przycisk_pomoc,0,0,160,40,makecol(120,120,120));   //przycisk z pomoca
        textout_ex(przycisk_pomoc,font,"Pomoc",0,przycisk_pomoc->h/2,CZARNY,-1);
        blit(przycisk_pomoc,bmp,0,0,pozycja_x_przyciskow,200,980,40);

        rectfill(przycisk_progowanie,0,0,160,40,makecol(120,120,120));//przycisk progowanie
        if (wczytano_obraz) textout_ex(przycisk_progowanie,font,"Progowanie",0,przycisk_progowanie->h/2,CZARNY,-1);
        else textout_ex(przycisk_progowanie,font,"Progowanie",0,przycisk_progowanie->h/2,makecol(150,150,150),-1);
        blit(przycisk_progowanie,bmp,0,0,pozycja_x_przyciskow,400,980,40);

        rectfill(przycisk_zmiana_jasnosci,0,0,160,40,makecol(120,120,120));//przycisk zmiana jasnosci
        if (wczytano_obraz) textout_ex(przycisk_zmiana_jasnosci,font,"Zmiana jasnosci",0,przycisk_zmiana_jasnosci->h/2,CZARNY,-1);
        else textout_ex(przycisk_zmiana_jasnosci,font,"Zmiana jasnosci",0,przycisk_zmiana_jasnosci->h/2,makecol(150,150,150),-1);
        blit(przycisk_zmiana_jasnosci,bmp,0,0,pozycja_x_przyciskow,460,980,40);

        rectfill(przycisk_korekcja_gamma,0,0,160,40,makecol(120,120,120));//przycisk korekcja gamma
        if (wczytano_obraz) textout_ex(przycisk_korekcja_gamma,font,"Korekcja gamma",0,przycisk_korekcja_gamma->h/2,CZARNY,-1);
        else textout_ex(przycisk_korekcja_gamma,font,"Korekcja gamma",0,przycisk_korekcja_gamma->h/2,makecol(150,150,150),-1);
        blit(przycisk_korekcja_gamma,bmp,0,0,pozycja_x_przyciskow,520,980,40);
}

//---------------Pomoc do programu------------------
int pomoc() {
    BITMAP *pomoc;
    pomoc=create_bitmap(800,600);
    int ktora_strona=1;
    char pom[2];
    char ktora_strona_str[50];

    while(!key[KEY_X]) {//dopoki uzytkowanik nie wcisnie X
        //ustawianie odpowiedniej strony pomocy
        itoa(ktora_strona,pom,10);
        strcpy(ktora_strona_str,"img/");
        strcat(ktora_strona_str,pom);
        strcat(ktora_strona_str,".bmp");

        pomoc=load_bitmap(ktora_strona_str,default_palette);
        //pomoc=load_bitmap("img/1.bmp",default_palette);
        if (!pomoc) allegro_message("Nie mozna zaladowac pliku graficznego z trescia pomocy!");
        else {
        blit(pomoc,bmp,0,0,0,0,800,600);
        blit(bmp,screen,0,0,0,0,screen->w,screen->h);
        }

        //if (key[KEY_ESC]) return 1;
        if (key[KEY_LEFT] || key[KEY_UP]) {
            if (ktora_strona < liczba_stron_pomocy) {
            ktora_strona++;
            }
            else ktora_strona=1;
        }
        if (key[KEY_RIGHT] || key[KEY_DOWN]) {
            if (ktora_strona > 1) {
            ktora_strona--;
            }
            else ktora_strona=liczba_stron_pomocy;
        }
        destroy_bitmap(pomoc);
        rest(50);
    }
    //destroy_bitmap(przycisk_pomoc);

    return 0;
}

//===============Program glowny=================
int main(int argc, char **argv) {

   // char *sciezka;
    int i=0;

    inicjalizacja();
        show_mouse(screen); //pokazanie kursora myszy na ekranie
        unscare_mouse();

    /*Glowna petla programu  */
    while (!key[KEY_ESC]) {
        //ustawianie na false, zeby raz wcisniety przycisk nie uruchamial sie bez przerwy
        kliknieto_przycisk_otworz=false;
        kliknieto_przycisk_zapisz=false;
        kliknieto_przycisk_progowanie=false;
        kliknieto_przycisk_zmiana_jasnosci=false;
        kliknieto_przycisk_korekcja_gamma=false;
        kliknieto_przycisk_pomoc=false;
        wcisniety_lpm=false;


        rectfill(bmp,0,0,screen->w,screen->h,makecol(100,100,100));
        //if (!wczytano_obraz)
        rectfill(bmp,0,0,800,600,makecol(50,50,50));

        przyciski();//rysowanie przyciskow

        if (wczytano_obraz) blit(obraz,bmp,0,0,0,0,800,600);

        if (mouse_b != 0 && czas>przedzial_czasu) {//jesli kliknieto jakis przycisk myszy
            if (mouse_b % 2 == 1) { //lpm
                    klik();
                    zaznaczenie(1);
            }
            if (mouse_b & 2) {//ppm
                zaznaczenie(2);
            }
        }
        czas++;

        rysuj_zaznaczenie();

        //--Obsluga klawiszy--

        if (key[KEY_O] || kliknieto_przycisk_otworz) {   //otwarcie pliku
            otworz();
        }

        if ((key[KEY_S] || kliknieto_przycisk_zapisz) && wczytano_obraz) {  //zapisanie pliku
            zapisz();
        }

        if (key[KEY_C] && czas>przedzial_czasu) {   //zmiana glebi obrazu\wyswietlenie glebii kolorow
            /*czas=0;
            if (glebia_kolorow == 24)
                glebia_kolorow=8;
            else
                glebia_kolorow=24;

            set_color_depth(glebia_kolorow);*/
            //set_color_conversion(COLORCONV_NONE);
            set_color_conversion(COLORCONV_REDUCE_TRUE_TO_HI);
            allegro_message("%d", bitmap_color_depth(obraz));
        }

        if (key[KEY_R] && czas>przedzial_czasu) {   //zaladowanie obrazu od nowa/zresetowanie bitmapy
            czas=0;
            obraz=load_bmp(nazwa_pliku,default_palette);
        }

        if ((key[KEY_P] || kliknieto_przycisk_progowanie) && wczytano_obraz) {   //progrowanie
            progowanie();
        }

        if ((key[KEY_J] || kliknieto_przycisk_zmiana_jasnosci) && wczytano_obraz) {   //progrowanie
            zmiana_jasnosci();
        }

        if ((key[KEY_G] || kliknieto_przycisk_korekcja_gamma) && wczytano_obraz) {   //progowanie
            korekcja_gamma();
        }

        if (key[KEY_F1] || kliknieto_przycisk_pomoc) {  //pomoc do programu
            pomoc();
        }

        //wyswietlenie wspolrzednych i wcisnietych przyciskow myszy
        textprintf_ex(bmp, font,screen->w-100,screen->h-20, makecol(0,0,0), - 1, "%d,%d,kl=%d", mouse_x,mouse_y,mouse_b);

        blit(bmp,screen,0,0,0,0,screen->w,screen->h);
        rest(5);
        //readkey();
    }

    zakoncz();

    return 0;
}

END_OF_MAIN()
