#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <bits/stdc++.h>

using namespace std;
struct hecho //estructura de un elemento "hecho" de la base de hechos
{
    string literal;
    float FC;
};

struct regla //estructura de un elemento "regla" de la base de conocimientos
{
    int identificador;
    vector<string> literales;
    string lit_consecuente;
    float FC;
    int dis_conjuncion = 2; //si este entero es 0 será una disyunción, si es 1 será una conjunción.
};

//esta función se usa para el obtener el output correcto, claro y comprensible acerca de la activación de la red y cada caso de inferencia
void printcaso1(int R, int dis_conjuncion, ofstream &output, vector<string> aux)
{

    if (dis_conjuncion == 0)
    {
        output << "Caso 1, Combinación de antecedentes de R" << R << "\n\tFC(";
        for (size_t i = 0; i < aux.size(); i++)
        {
            output << aux[i];
            if ((i + 1) != aux.size())
                output << " u ";
        }
        output << ") = max(";
    }
    else
    {
        output << "Caso 1, Combinación de antecedentes de R" << R << "\n\tFC(";
        for (size_t i = 0; i < aux.size(); i++)
        {
            output << aux[i];
            if ((i + 1) != aux.size())
                output << " n ";
        }
        output << ") = min(";
    }

    for (size_t i = 0; i < aux.size(); i++)
    {
        output << "FC(" << aux[i] << ")";
        if ((i + 1) != aux.size())
            output << ",";
    }
    output << ")";
}

//esta función se usa para el obtener el output correcto, claro y comprensible acerca de la activación de la red y cada caso de inferencia
void printcaso2(ofstream &output, string meta, vector<int> aux, float fccaso2)
{
    output << "Caso 2, Combinación de las reglas";
    for (size_t i = 0; i < aux.size(); i++)
    {
        output << " R" << aux[i] << " ";
    }
    output << "\n";
    output << "\tFC(" << meta << ") = " << fccaso2 << "\n\n";
}

//Función que comprueba si el literal pasado como "meta" se encuentra en la BH
int contenido(string meta, vector<hecho> hechos, int nhechos, float &fcfinal)
{
    for (int i = 0; i < nhechos; i++) //recorriendo todos los hechos
    {

        if (hechos[i].literal == meta)
        {
            fcfinal = hechos[i].FC; //convertirá "fcfinal" en el valor del FC de este hecho.

            return 1;
        }
    }
    return 0;
}

//Función que comprueba cuales son las reglas cuyo consecuente es la meta actual
void Equiparar(string meta, vector<regla> reglas, int nreglas, vector<int> &ConjConf)
{
    for (int i = 0; i < nreglas; i++)
    {

        if (reglas[i].lit_consecuente == meta)
        {
            ConjConf.push_back(reglas[i].identificador); //Y añade al Conjunto Conflicto estas reglas.
        }
    }
}

//Función que comprueba para la regla "rule" cuales son sus antecedentes y los añade a "NuevasMetas"
void Antecedentes(int rule, vector<regla> reglas, vector<string> &NuevasMetas, int nreglas, int &dis_conj)
{
    for (int i = 0; i < nreglas; i++)
    {
        if (reglas[i].identificador == rule)
        {
            for (size_t j = 0; j < reglas[i].literales.size(); j++)
            {
                NuevasMetas.push_back(reglas[i].literales[j]);
            }
            dis_conj = reglas[i].dis_conjuncion;
            break;
        }
    }
}

//Método en el que simplemente aplicamos el caso 1 aprendido en clase a los antecedentes pasados en el argumento, y devolveremos el FC obtenido
float aplicarCaso1(int rule, vector<float> antecedentes, int dis_conj)
{
    switch (dis_conj)
    {
    case 0:
    {
        float maximo = -1;
        for (int i = 0; i < int(antecedentes.size()); i++)
        {
            if (antecedentes[i] > maximo)
                maximo = antecedentes[i];
        }
        return maximo;
        break;
    }
    case 1:
    {
        float minimo = 1;
        for (int i = 0; i < int(antecedentes.size()); i++)
        {
            if (antecedentes[i] < minimo)
                minimo = antecedentes[i];
        }
        return minimo;
        break;
    }
    default:
        return antecedentes[0];
        break;
    }
}

//Lo mismo para el caso 3, aplicamos el caso 3 combinando dos evidencias y devolviendo el FC resultado de esta operación.
float aplicarCaso3(int rule, float caso1, vector<regla> reglas, int nreglas)
{
    float fcregla;
    for (int i = 0; i < nreglas; i++)
    {
        if (reglas[i].identificador == rule)
        {
            fcregla = reglas[i].FC;
        }
    }
    fcregla = ((fcregla) * (max((float)0.0, caso1)));
    return fcregla;
}

//Para aquellas hipótesis que tienen más de una evidencia para ella misma les aplicaremos el caso 2 visto en clase.
float aplicarCaso2(vector<float> &FCCaso3)
{
    float FC1, FC2;
    FC1 = FCCaso3.back();
    FCCaso3.pop_back();
    FC2 = FCCaso3.back();
    FCCaso3.pop_back();
    if (((FC1 < 0) && (FC2 > 0)) || ((FC1 > 0) && (FC2 < 0)))
        return ((FC1 + FC2) / (1 - min(abs(FC1), abs(FC2))));

    else if ((FC1 >= 0) && (FC2 >= 0))
        return (FC1 + (FC2 * (1 - FC1)));
    else
        return (FC1 + (FC2 * (1 + FC1)));
}

//Esta es la función principal y en la que se llaman al resto.

float verificar(string meta, vector<hecho> hechos, vector<regla> reglas, int nhechos, int nreglas, ofstream &output)
{
    float fcfinal;
    vector<int> ConjConf;
    int caso = 0;
    if (contenido(meta, hechos, nhechos, fcfinal)) //Si está la meta en la BH devolvemos su FC.
    {
        return fcfinal;
    }

    else
        Equiparar(meta, reglas, nreglas, ConjConf); //Si no lo está, obtenemos todas las reglas que generan esta meta

    if (int(ConjConf.size()) >= 2) //Si hay una o más reglas, tendremos que aplicar el caso2 próximamente!
        caso = 2;

    vector<int> aux = ConjConf;
    vector<float> FCCaso3(ConjConf.size());
    int contador = 0;

    while (!ConjConf.empty()) //Mientras que haya reglas en el conjunto conflicto
    {
        int R = ConjConf.back(); //"Elegimos" una de ellas
        ConjConf.pop_back();
        vector<string> NuevasMetas;
        int dis_conjuncion;
        Antecedentes(R, reglas, NuevasMetas, nreglas, dis_conjuncion); //Obtenemos los antecedentes de esta regla y los añadimos a NuevasMetas
        vector<string> aux = NuevasMetas;
        vector<float> antecedentes;  //vector que usaremos para guardar el FC de cada antecedente.
        while (!NuevasMetas.empty()) //Mientras que haya antecedentes de la regla
        {
            string Nmet = NuevasMetas.back();
            NuevasMetas.pop_back();
            float verificado = verificar(Nmet, hechos, reglas, nhechos, nreglas, output); //Verificamos recursivamente cada uno de ellos
            antecedentes.push_back(verificado);
        }
        float FCCaso1 = aplicarCaso1(R, antecedentes, dis_conjuncion); //Aplicamos el caso 1 a los antecedentes anteriores.
        printcaso1(R, dis_conjuncion, output, aux);                    //printeamos en el fichero de salida información del proceso de inferencia
        output << " = " << FCCaso1 << "\n\n";
        //Añadimos seguidamente el resultado de aplicar el caso 3 a la regla que estuviesemos tratando en el Conjunto Conflicto
        FCCaso3[contador] = aplicarCaso3(R, FCCaso1, reglas, nreglas);
        output << "Caso 3,Combinación de la evidencia con la regla R" << R << "\n";
        output << "\tFC(" << meta << ")=FC(R" << R << "*max(0,FCCaso1 anterior(" << FCCaso1 << ")) = " << FCCaso3[contador] << "\n\n";
        contador++;
    }
    if (caso == 2) //Una vez hayamos terminado de tratar las reglas, observamos si tenemos que aplicar el caso 2.
    {
        float fccaso2;
        while (FCCaso3.size() >= 2)
        {
            fccaso2 = aplicarCaso2(FCCaso3); //Y lo vamos aplicando al resultado de cada caso 3 para cada regla(de 2 en 2)
            FCCaso3.push_back(fccaso2);
        }
        printcaso2(output, meta, aux, fccaso2);
        return fccaso2;
    }
    else
        return FCCaso3[0];
}

int main(int argc, char **argv)
{
    string c;
    string h;

    if (argc <= 2 || argc > 3)
    {
        fprintf(stderr, "HELP\nEJECUCIÓN DEL PROGRAMA: %s BC BH \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    c = argv[1];
    h = argv[2];
    ifstream BC(c);
    if (BC.fail())
    {
        fprintf(stderr, "El fichero de la base de conocimientos indicado no es correcto\n");
        exit(EXIT_FAILURE);
    }
    string linea;
    getline(BC, linea); //obtenemos la primera linea, que corresponde al numero de reglas que habrá.
    char *ch = const_cast<char *>(linea.c_str());
    int nreglas = atoi(ch);
    int nlinea = -1;
    string palabra;
    vector<regla> reglas(nreglas);
    for (int i = 0; i < nreglas; i++) //leeremos tantas lineas como reglas haya
    {
        getline(BC, linea);
        nlinea++;
        int si = 0;
        int entonces = 0;
        int y = 0;
        int o = 0;
        istringstream iss(linea);
        regla reglax;
        reglax.identificador = nlinea + 1;
	//La siguiente variable nos servirá para detectar las palabras "Si","Entonces", "y" y "o" ya que en el guión no pone que sean reservadas, así que podría darse una regla tal que "Si Si entonces B" o "Si A Entonces Entonces", no sería lo normal, pero mejor prevenir eso. 
        int reservada = 0;			
        while (iss >> palabra)
        {
            switch (reservada)
            {
            case 0:
            {
                if (palabra == "Si") //Si la palabra de la linea es un "Si" sabemos que la siguiente palabra será un literal.
                {
                    si = 1;	     //Avisamos que en la siguiente palabra hay "datos que recoger", con la variable "Si" y "reservada"	
                    reservada = 1;   
                    continue;
                }

                else if (palabra == "y") //De igual forma que con el "Si"
                {
                    y = 1;
                    reservada = 1;
                    continue;
                }

                else if (palabra == "o") //etc.
                {
                    o = 1;
                    reservada = 1;
                    continue;
                }
                else if (palabra == "Entonces") //etc.
                {
                    entonces = 1;
                    reservada = 1;
                    continue;
                }
                else if (palabra.substr(0, 3) == "FC=") //Si nos encontramos con la palabra YA RESERVADA "FC=" guardamos el valor del FC!
                {
                    reglax.FC = stof(palabra.substr(3).c_str()); //nos quedamos con la cadena a partir del tercer caracter.
                    continue;
                }
                break;
            }
            default:		//si "reservada" es diferente de 0, a.k.a 1.
                if (si)		//Si la palabra anterior fue un "Si" ahora viene un literal
                {
                    reglax.literales.push_back(palabra); 	//Lo añadimos.
                    si = 0;
                }

                else if (y)	//igual con un "y"
                {
                    reglax.literales.push_back(palabra);	//Lo añadimos
                    y = 0;
                    reglax.dis_conjuncion = 1;
                }
                else if (o)	//y con un "o"
                {	
                    reglax.literales.push_back(palabra);
                    o = 0;
                    reglax.dis_conjuncion = 0;
                }
                else if (entonces)
                {
                    reglax.lit_consecuente = palabra.substr(0, palabra.size() - 1); //Añadimos el consecuente, sin la coma del final
                    entonces = 0;
                }
                reservada = 0;	//Volvemos a poner reservada a 0!
                break;
            }
        }
        reglas[nlinea] = reglax;	//Añadimos la regla obtenida.
    }
    BC.close();

    ifstream BH(h);
    if (BH.fail())
    {
        fprintf(stderr, "El fichero de la base de hechos indicado no es correcto\n");
        exit(EXIT_FAILURE);
    }
    string lineax;
    getline(BH, lineax);
    char *chh = const_cast<char *>(lineax.c_str());
    int nhechos = atoi(chh);		//obtenemos de la primera linea cuantos hechos van a haber
    nlinea = -1;
    string palabrax;
    vector<hecho> hechos(nhechos);
    int objetivo = 0;
    string objetivox;
    while (getline(BH, lineax)) //nos limitamos a leer líneas hasta el final del fichero/el objetivo, en el que haremos un break;
    {
        int cont = 0;
        hecho hechox;
        if (objetivo)
        {
            objetivox = lineax;		//Esta es la línea en la que está el objetivo!
            break;			//sabemos que después no hay nada que nos interese, hacemos break;
        }
        nlinea++;
        istringstream iss(lineax);
        while (iss >> palabrax)
        {
            if (cont)		//para que entre en el if, debe de haber tratado el literal antes y haber hecho el cont++ como veremos.
            {
                hechox.FC = stof(palabrax.substr(3).c_str()); //Y entonces trata el FC del literal y lo guarda
                continue;
            }
            else if (palabrax == "Objetivo") //si la palabra es Objetivo(palabra reservada) pues sabemos que la siguiente linea es el objetivo
            {
                objetivo = 1;
                continue;
            }
            else
            {
                hechox.literal = palabrax.substr(0, palabrax.size() - 1); //si no, pues es un literal, y así lo guardaremos, eliminando la "," final, claro.
                cont++;
            }
        }
        if (!objetivo)	//si la ultima palabra que hemos tratado no era "objetivo" significa que hemos tratado un hecho, así que lo guardamos.
            hechos[nlinea] = hechox;
    }
    BH.close();
    ofstream output;
    output.open(c + "__" + h);
    output << "El fichero de la BC es: " << c << " y el de la BH es: " << h << endl
           << endl;
    verificar(objetivox, hechos, reglas, nhechos, nreglas, output);
    output.close();
    exit(EXIT_SUCCESS);
}
