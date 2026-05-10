# SPESE-E-BUDGET
PROGETTO FINALE DI PROGRAMMAZIONE: SISTEMA DI GESTIONE DELLE SPESE PERSONALI E DEL BUDGET

Nome: Alessandro
Cognome: Citran
Matricola: 0082500267 
Corso di Laurea: L-8 - Ingegneria Informatica e Intelligenza artificiale 
Esame: FONDAMENTI DI INFORMATICA - 0082509INGINF05V

Elaborato: EsameDB
Linguaggio scelto: C++ e postgreSQL
Programma utilizzato: Embarcadero Dev-C++ e PgADMIN

8.1. Script SQL
a) Script di creazione del database
Creazione dello schema:

CREATE SCHEMA SpeseBudget

Istruzioni creazione della prima tabella:
SET search_path TO spesebudget;
CREATE TABLE Categorie (
    id_categoria SERIAL PRIMARY KEY,
    nome VARCHAR(50) UNIQUE
);

Istruzioni creazione delle altre tabelle:
SET search_path TO spesebudget;
CREATE TABLE Spese (
    id_spesa SERIAL PRIMARY KEY,
    data DATE NOT NULL,
    importo DECIMAL(10, 2) NOT NULL CHECK (importo > 0),
    descrizione VARCHAR(255),
    id_categoria INTEGER NOT NULL,
    FOREIGN KEY (id_categoria) REFERENCES Categorie (id_categoria) ON DELETE CASCADE
);
CREATE TABLE Budget (
    id_budget SERIAL PRIMARY KEY,
    mese_riferimento CHAR(7) NOT NULL, -- Formato 'YYYY-MM'
    importo_limite DECIMAL(10, 2) NOT NULL CHECK (importo_limite > 0),
    id_categoria INTEGER NOT NULL,
    UNIQUE(mese_riferimento, id_categoria),
    FOREIGN KEY (id_categoria) REFERENCES Categorie (id_categoria) ON DELETE CASCADE
);

b) Script di inserimento dati di esempio
Istruzioni per fare esempio di insert:
INSERT INTO spesebudget.categorie (id_categoria, nome) VALUES 
(1,'Cibo'), 
(2,'Trasporti'), 
(3,'Svago');

