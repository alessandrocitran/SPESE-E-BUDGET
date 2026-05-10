#include <iostream>
#include <string>
#include <libpq-fe.h>
#include <windows.h> 

using namespace std;


// Apertura Programma per il benvenuto


void mostraIntestazione() {
    system("cls");
    cout << "-------------------------" << endl;
    cout << " SISTEMA SPESE PERSONALI " << endl;
    cout << "-------------------------" << endl;
}

void mostraBenvenuto() {
    system("color 0B"); // Sfondo nero, testo azzurro
    system("cls");
    cout << "=================================================" << endl;
    cout << "       BENVENUTO NEL TUO GESTORE SPESE           " << endl;
    cout << "           'MyPersonalBudget v1.0'               " << endl;
    cout << "=================================================" << endl;
    cout << "\n  Organizza le tue finanze in modo semplice!" << endl;
    cout << "  Connessione al database in corso..." << endl;
    cout << "-------------------------------------------------" << endl;
    Sleep(3000); // Attesa per far aprire il menù principale
}

void attendi() {
    cout << "\nPremi un tasto per continuare...";
    system("pause > nul");
}


// PROGRAMMA PRINCIPALE


int main() {
    mostraBenvenuto();

    // CONNESSIONE a PgAdmin
    string conn_str = "host=localhost dbname=postgres user=postgres password=Aleele03 port=5432";
    PGconn *conn = PQconnectdb(conn_str.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        system("color 0C"); // Colora di rosso se vai in errore
        cerr << "\n [ERRORE]: Impossibile connettersi al database." << endl;
        cerr << " Messaggio: " << PQerrorMessage(conn) << endl;
        PQfinish(conn);
        system("pause");
        return 1;
    }

    // Impostazione automatica dello schema
    PQclear(PQexec(conn, "SET search_path TO spesebudget;"));

    int scelta;
    bool inEsecuzione = true;

    while (inEsecuzione) {
        mostraIntestazione();
        cout << "1. Gestione Categorie" << endl;
        cout << "2. Inserisci Spesa" << endl;
        cout << "3. Definisci Budget Mensile" << endl;
        cout << "4. Visualizza Report" << endl;
        cout << "5. Esci" << endl;
        cout << "-------------------------" << endl;
        cout << "Inserisci la tua scelta: ";
        cin >> scelta;

        switch (scelta) {
            
            case 1: { // MODULO 1: GESTIONE CATEGORIE
                
                int sceltaSottoMenu;
                string nomeCat;
                string nuovoNome;
                bool tornaAlMenu = false;

                while (!tornaAlMenu) {
                    mostraIntestazione();
                    cout << "--- GESTIONE CATEGORIE ---" << endl;

                    // 1. VISUALIZZAZIONE LISTA AGGIORNATA
                    PGresult *resL = PQexec(conn, "SELECT nome FROM categorie ORDER BY nome ASC;");
                    cout << "Categorie attuali:" << endl;
                    
                    if (PQresultStatus(resL) == PGRES_TUPLES_OK) {
                        int nCat = PQntuples(resL);
                        if (nCat == 0) {
                            cout << " (Nessuna categoria presente)" << endl;
                        } else {
                            for (int i = 0; i < nCat; i++) {
                                cout << " - " << PQgetvalue(resL, i, 0) << endl;
                            }
                        }
                    }
                    PQclear(resL); // Libera memoria lista
                    
                    cout << "------------------------------------------" << endl;
                    cout << "1) Aggiungi categoria" << endl;
                    cout << "2) Cancella categoria" << endl;
                    cout << "3) Rinomina categoria" << endl;
                    cout << "0) Torna al menu principale" << endl;
                    cout << "Scelta: ";
                    
                    if (!(cin >> sceltaSottoMenu)) {
                        cin.clear();
                        cin.ignore(1000, '\n');
                        continue;
                    }

                    if (sceltaSottoMenu == 0) {
                        tornaAlMenu = true;
                        continue;
                    }

                    // CREAZIONE SOTTOMENU 
                    
                    if (sceltaSottoMenu == 1) { // AGGIUNGI
                        cout << "\nNome nuova categoria: ";
                        cin >> ws; getline(cin, nomeCat);

                        // Controllo duplicati
                        string sqlCheck = "SELECT COUNT(*) FROM categorie WHERE nome = '" + nomeCat + "';";
                        PGresult *resCheck = PQexec(conn, sqlCheck.c_str());
                        int esiste = atoi(PQgetvalue(resCheck, 0, 0));
                        PQclear(resCheck);

                        if (esiste > 0) {
                            cout << ">> ERRORE: La categoria '" << nomeCat << "' esiste gia'!" << endl;
                        } else {
                            string sqlInsert = "INSERT INTO categorie (nome) VALUES ('" + nomeCat + "');";
                            PGresult *resInsert = PQexec(conn, sqlInsert.c_str());
                            if (PQresultStatus(resInsert) == PGRES_COMMAND_OK) {
                                cout << ">> Successo: Categoria aggiunta correttamente." << endl;
                            }
                            PQclear(resInsert);
                        }
                        attendi();

                    } else if (sceltaSottoMenu == 2) { // CANCELLA
                        cout << "\nNome della categoria da eliminare: ";
                        cin >> ws; getline(cin, nomeCat);

                        // Recupero ID
                        string sqlID = "SELECT id_categoria FROM categorie WHERE nome = '" + nomeCat + "';";
                        PGresult *resID = PQexec(conn, sqlID.c_str());

                        if (PQntuples(resID) == 0) {
                            cout << ">> ERRORE: Categoria non trovata." << endl;
                            PQclear(resID);
                        } else {
                            string id_cat = PQgetvalue(resID, 0, 0);
                            PQclear(resID);

                            // Controllo se ci sono spese collegate
                            string sqlSpese = "SELECT COUNT(*) FROM spese WHERE id_categoria = " + id_cat + ";";
                            PGresult *resSpese = PQexec(conn, sqlSpese.c_str());
                            int numSpese = atoi(PQgetvalue(resSpese, 0, 0));
                            PQclear(resSpese);

                            if (numSpese > 0) {
                                cout << ">> AZIONE NEGATA: Esistono " << numSpese << " spese per questa categoria." << endl;
                            } else {
                                string sqlDel = "DELETE FROM categorie WHERE id_categoria = " + id_cat + ";";
                                PQclear(PQexec(conn, sqlDel.c_str()));
                                cout << ">> Successo: Categoria eliminata." << endl;
                            }
                        }
                        attendi();

                    } else if (sceltaSottoMenu == 3) { // RINOMINA
                        cout << "\nNome categoria da modificare: ";
                        cin >> ws; getline(cin, nomeCat);

                        string sqlCheckV = "SELECT COUNT(*) FROM categorie WHERE nome = '" + nomeCat + "';";
                        PGresult *resV = PQexec(conn, sqlCheckV.c_str());
                        int esisteV = atoi(PQgetvalue(resV, 0, 0));
                        PQclear(resV);

                        if (esisteV == 0) {
                            cout << ">> ERRORE: La categoria non esiste." << endl;
                        } else {
                            cout << "Nuovo nome: ";
                            getline(cin, nuovoNome);

                            string sqlUpd = "UPDATE categorie SET nome = '" + nuovoNome + "' WHERE nome = '" + nomeCat + "';";
                            PGresult *resUpd = PQexec(conn, sqlUpd.c_str());
                            if (PQresultStatus(resUpd) == PGRES_COMMAND_OK) {
                                cout << ">> Successo: Categoria rinominata." << endl;
                            } else {
                                cout << ">> ERRORE: Nome gia' occupato o errore SQL." << endl;
                            }
                            PQclear(resUpd);
                        }
                        attendi();
                    }
                }
                break;
            }
            
            case 2: { // MODULO 2: INSERIMENTO SPESA
                mostraIntestazione();
                cout << "--- NUOVA SPESA ---" << endl;

                // Recuperiamo le categorie
                PGresult *resCat = PQexec(conn, "SELECT id_categoria, nome FROM categorie ORDER BY nome ASC;");
                int nCat = PQntuples(resCat);

                if (nCat == 0) {
                    cout << "Errore: Devi prima creare una categoria nel Modulo 1!" << endl;
                    PQclear(resCat);
                    attendi();
                    break;
                }

                // Visualizziamo un menu con numeri sequenziali (1, 2, 3...)
                cout << "Categorie disponibili:" << endl;
                for (int i = 0; i < nCat; i++) {
                    // i + 1 crea la numerazione pulita per l'utente
                    cout << "[" << i + 1 << "] " << PQgetvalue(resCat, i, 1) << endl;
                }
                cout << "[0] Torna indietro" << endl;

                int sceltaFittizia;
                cout << "\nSeleziona il numero della categoria: ";
                cin >> sceltaFittizia;

                if (sceltaFittizia == 0) {
                    PQclear(resCat);
                    break;
                }

                // Recuperiamo l'ID REALE corrispondente alla scelta dell'utente
                if (sceltaFittizia < 1 || sceltaFittizia > nCat) {
                    cout << "Scelta non valida!" << endl;
                    PQclear(resCat);
                    attendi();
                    break;
                }

                string id_reale = PQgetvalue(resCat, sceltaFittizia - 1, 0);
                string nome_cat = PQgetvalue(resCat, sceltaFittizia - 1, 1);
                PQclear(resCat); // Abbiamo l'ID, possiamo liberare la memoria

                // Inserimento della spesa usando id_reale
                string data, descrizione;
                double importo;

                cout << "Hai scelto: " << nome_cat << endl;
                cout << "Importo: "; cin >> importo;
                cout << "Data (AAAA-MM-DD): "; cin >> data;
                cout << "Descrizione: "; cin >> ws; getline(cin, descrizione);

                string sqlInsert = "INSERT INTO spese (id_categoria, importo, data, descrizione) VALUES (" 
                                   + id_reale + ", " + to_string(importo) + ", '" + data + "', '" + descrizione + "');";
                
                PGresult *resIns = PQexec(conn, sqlInsert.c_str());
                if (PQresultStatus(resIns) == PGRES_COMMAND_OK) {
                    cout << "\n>> Spesa registrata con successo!" << endl;
                } else {
                    cout << "\n>> Errore: " << PQerrorMessage(conn) << endl;
                }
                
                PQclear(resIns);
                attendi();
                break;
            }
            
            case 3: { // MODULO 3: DEFINISCI BUDGET MENSILE
                string mese, nomeCat, limite_str;
                double limite_val;
                string id_cat;
                bool annulla = false;

                mostraIntestazione();
                cout << "--- IMPOSTA BUDGET MENSILE (0 per uscire) ---" << endl;

                // MESE (YYYY-MM)
                do {
                    cout << "\nMese di riferimento (AAAA-MM) [0 per uscire]: ";
                    cin >> mese;
                    if (mese == "0") { annulla = true; break; }
                    
                    // Validazione formato AAAA-MM
                    if (mese.length() == 7 && mese[4] == '-') {
                        break;
                    } else {
                        cout << ">> Errore: formato data non corretto. Usa AAAA-MM." << endl;
                    }
                } while (true);
                if (annulla) break;

                // INPUT NOME CATEGORIA E VERIFICA ESISTENZA
                do {
                    // Mostra le categorie esistenti
                    cout << "\nCategorie disponibili nel sistema:" << endl;
                    PGresult *resLista = PQexec(conn, "SELECT nome FROM categorie ORDER BY nome;");
                    for (int i = 0; i < PQntuples(resLista); i++) {
                        cout << " - " << PQgetvalue(resLista, i, 0) << endl;
                    }
                    PQclear(resLista);

                    cout << "\nInserisci il NOME della categoria [0 per uscire]: ";
                    cin.ignore(1000, '\n');
                    getline(cin, nomeCat);

                    if (nomeCat == "0") { annulla = true; break; }

                    // Cerca l'ID corrispondente al nome inserito
                    string sqlCheck = "SELECT id_categoria FROM categorie WHERE nome = '" + nomeCat + "';";
                    PGresult *resCheck = PQexec(conn, sqlCheck.c_str());

                    if (PQntuples(resCheck) > 0) {
                        id_cat = PQgetvalue(resCheck, 0, 0); 
                        PQclear(resCheck);
                        break;
                    } else {
                        cout << ">> Errore: la categoria '" << nomeCat << "' non esiste." << endl;
                        PQclear(resCheck);
                    }
                } while (true);
                if (annulla) break;

                // IMPORTO BUDGET che deve essere > 0
                do {
                    cout << "\nImporto del budget [0 per uscire]: ";
                    cin >> limite_str;
                    if (limite_str == "0") { annulla = true; break; }

                    try {
                        limite_val = stod(limite_str);
                        if (limite_val > 0) {
                            break;
                        } else {
                            cout << ">> Errore: il budget deve essere maggiore di zero." << endl;
                        }
                    } catch (...) {
                        cout << ">> Errore: inserisci un numero valido." << endl;
                    }
                } while (true);
                if (annulla) break;

                // ELABORAZIONE: INSERIMENTO O AGGIORNAMENTO del nuovo BUDGET
                
                string sqlUpsert = "INSERT INTO budget (mese_riferimento, importo_limite, id_categoria) "
                                   "VALUES ('" + mese + "', " + to_string(limite_val) + ", " + id_cat + ") "
                                   "ON CONFLICT (mese_riferimento, id_categoria) "
                                   "DO UPDATE SET importo_limite = EXCLUDED.importo_limite;";

                PGresult *resFinal = PQexec(conn, sqlUpsert.c_str());

                if (PQresultStatus(resFinal) == PGRES_COMMAND_OK) {
                    cout << "\nMessaggio di successo: Budget mensile salvato correttamente." << endl;
                } else {
                    cout << "\nErrore durante il salvataggio: " << PQerrorMessage(conn) << endl;
                }
                PQclear(resFinal);

                attendi();
                break;
            }
            
            case 4: { // MODULO 4: MENU DEI REPORT
                int sceltaReport;
                bool tornaPrincipale = false;

                do {
                    mostraIntestazione();
                    cout << "--- MENU DEI REPORT ---" << endl;
                    cout << "1. Totale spese per categoria" << endl;
                    cout << "2. Spese mensili vs Budget" << endl;
                    cout << "3. Elenco completo spese (ordinate per data)" << endl;
                    cout << "4. Ritorna al menu principale" << endl;
                    cout << "-----------------------" << endl;
                    cout << "Inserisci la tua scelta: ";
                    cin >> sceltaReport;

                    switch (sceltaReport) {
                        
                        case 1: { // REPORT: TOTALE DELLE SPESE PER CATEGORIA (Includendo gli zeri)
                            mostraIntestazione();
                            cout << "--- REPORT 1 - TOTALE SPESE PER CATEGORIA ---" << endl;
                            
                        
                            string sql = "SELECT c.nome, COALESCE(SUM(s.importo), 0) "
                                         "FROM categorie c "
                                         "LEFT JOIN spese s ON c.id_categoria = s.id_categoria "
                                         "GROUP BY c.nome "
                                         "ORDER BY c.nome ASC;"; // Ordinamento alfabetico
                            
                            PGresult *res = PQexec(conn, sql.c_str());

                            if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                                int righe = PQntuples(res);
                                
                                if (righe == 0) {
                                    cout << "\nNessuna categoria presente nel sistema." << endl;
                                } else {
                                    cout << "\n" << endl;
                                    printf("%-20s %15s\n", "Categoria", "Totale Speso");
                                    cout << "------------------------------------------------" << endl;

                                    for (int i = 0; i < righe; i++) {
                                        string nomeCat = PQgetvalue(res, i, 0);
                                        double totale = atof(PQgetvalue(res, i, 1));

                                        printf("%-20s %12.2f EUR\n", nomeCat.c_str(), totale);
                                    }
                                    cout << "------------------------------------------------" << endl;
                                }
                            } else {
                                cout << "Errore SQL: " << PQerrorMessage(conn) << endl;
                            }
                            
                            PQclear(res);
                            attendi();
                            break;
                        }
                        
                        case 2: { // SPESE MENSILI VS BUDGET
                            mostraIntestazione();
                            string meseRichiesto;
                            bool annulla = false;

                            cout << "--- REPORT 2: SPESE VS BUDGET ---" << endl;
                            cout << "Inserisci il mese (AAAA-MM) [0 per tornare indietro]: ";
                            cin >> meseRichiesto;

                            if (meseRichiesto == "0") break;

                            // QUERY SQL: Unisce budget e spese effettive per il mese scelto
                            string sql = 
                                "SELECT c.nome, "
                                "       b.importo_limite, "
                                "       COALESCE(SUM(s.importo), 0) AS totale_speso "
                                "FROM budget b "
                                "JOIN categorie c ON b.id_categoria = c.id_categoria "
                                "LEFT JOIN spese s ON c.id_categoria = s.id_categoria "
                                "AND TO_CHAR(s.data, 'YYYY-MM') = '" + meseRichiesto + "' "
                                "WHERE b.mese_riferimento = '" + meseRichiesto + "' "
                                "GROUP BY c.nome, b.importo_limite;";

                            PGresult *res = PQexec(conn, sql.c_str());

                            if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                                int righe = PQntuples(res);
                                if (righe == 0) {
                                    cout << "\nNessun budget impostato per il mese: " << meseRichiesto << endl;
                                } else {
                                    cout << "\n--- RISULTATI PER IL MESE: " << meseRichiesto << " ---" << endl;
                                    
                                    for (int i = 0; i < righe; i++) {
                                        string categoria = PQgetvalue(res, i, 0);
                                        double budget = atof(PQgetvalue(res, i, 1));
                                        double speso = atof(PQgetvalue(res, i, 2));

                                        cout << "\nCategoria: " << categoria << endl;
                                        cout << "Budget: " << budget << endl;
                                        cout << "Speso: " << speso << endl;

                                        // CONFRONTO
                                        cout << "Stato: ";
                                        if (speso > budget) {
                                            cout << "SUPERAMENTO BUDGET" << endl;
                                        } else if (speso == budget) {
                                            cout << "BUDGET AL LIMITE" << endl;
                                        } else {
                                            cout << "SPESE INFERIORI AL BUDGET" << endl;
                                        }
                                        cout << "-----------------------------------" << endl;
                                    }
                                }
                            } else {
                                cout << "Errore SQL: " << PQerrorMessage(conn) << endl;
                            }

                            PQclear(res);
                            attendi();
                            break;
                        }
                        
                        case 3: { //ELENCO COMPLETO ORDINATO PER DATA E CATEGORIA
                            mostraIntestazione();
                            cout << "--- REPORT 3 - ELENCO COMPLETO SPESE ---" << endl;

                            // Ordinamento prima per data (recente -> vecchia) e poi per nome categoria
                            string sql = "SELECT s.data, c.nome, s.importo, s.descrizione "
                                         "FROM spese s "
                                         "JOIN categorie c ON s.id_categoria = c.id_categoria "
                                         "ORDER BY s.data DESC, c.nome ASC;";

                            PGresult *res = PQexec(conn, sql.c_str());

                            if (PQresultStatus(res) == PGRES_TUPLES_OK) {
                                int righe = PQntuples(res);
                                
                                if (righe == 0) {
                                    cout << "\nNessuna spesa presente nel database." << endl;
                                } else {
                                    cout << "\n" << endl;
                                    // Intestazione Colonne
                                    printf("%-12s %-18s %-12s %s\n", "Data", "Categoria", "Importo", "Descrizione");
                                    cout << "----------------------------------------------------------------------" << endl;

                                    for (int i = 0; i < righe; i++) {
                                        string data = PQgetvalue(res, i, 0);
                                        string categoria = PQgetvalue(res, i, 1);
                                        double importo = atof(PQgetvalue(res, i, 2));
                                        string descrizione = PQgetvalue(res, i, 3);

                                        // Formattazione della riga
         
                                        printf("%-12s %-18s %8.2f EUR   %s\n", 
                                               data.c_str(), 
                                               categoria.c_str(), 
                                               importo, 
                                               descrizione.c_str());
                                    }
                                    cout << "----------------------------------------------------------------------" << endl;
                                }
                            } else {
                                cout << "Errore SQL: " << PQerrorMessage(conn) << endl;
                            }

                            PQclear(res);
                            attendi();
                            break;
                        }

                        case 4: // RITORNA al menu
                            tornaPrincipale = true;
                            break;

                        default:
                            cout << "\nScelta non valida. Riprova." << endl;
                            Sleep(1000);
                            break;
                    }

                } while (!tornaPrincipale);
                
                break; // Esce dal case 4 del menu principale
            }
         
            

            case 5:
                cout << "\nUscita... Grazie per aver usato MySpeseBudget!" << endl;
                inEsecuzione = false;
                break;

            default:
                cout << "\nScelta non valida. Riprovare." << endl;
                Sleep(1500);
                break;
        }
    }

    PQfinish(conn);
    return 0;
}
