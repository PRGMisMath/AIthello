#include "Terminal.hpp"


// D�coupe une cha�ne en mots, en gardant les cha�nes entre guillemets comme un seul mot
inline std::list<std::string> split(const std::string& ligne) {
    std::list<std::string> result;
    std::string mot;
    bool in_quotes = false;
    for (size_t i = 0; i < ligne.size(); ++i) {
        char c = ligne[i];
        if (c == '"') {
            in_quotes = !in_quotes;
            continue;
        }
        if (std::isspace(c) && !in_quotes) {
            if (!mot.empty()) {
                result.push_back(mot);
                mot.clear();
            }
        }
        else {
            mot += c;
        }
    }
    if (in_quotes) {
        throw ParseException("Guillemets non fermes dans la commande.");
    }
    if (!mot.empty()) {
        result.push_back(mot);
    }
    return result;
}

void Terminal::run() {
    std::string input;
    while (state.alive) {
        (*state.fluxOut) << "> ";
        state.fluxOut->flush();
        std::getline(*state.fluxIn, input);
        // Gestion des exceptions de parsing
        try {
            this->parse(input, state);
            (*state.fluxOut) << std::endl;
        }
        catch (const ParseException& e) {
            (*state.fluxOut) << "Erreur de parsing : " << e.what() << std::endl;
        }
    }
}


void Terminal::parse(const std::string& ligne, TerminalState& state) {
    std::list<std::string> mots = split(ligne);
    if (mots.empty()) return;
    auto it = commandes.find(mots.front());
    mots.pop_front();
    if (it != commandes.end()) {
        CmdFormat cmd = it->second;

        // Reinitialisation des etats des arguments
        for (BaseParam* param : cmd.params_opt) {
            param->reset();
        }
        for (BaseParam* param : cmd.params_req) {
            param->reset();
        }

        // Gestion (et suppression) des arguments optionnels
        for (BaseParam* param : cmd.params_opt) {
            if (mots.empty()) {
                break;
            }
            auto it_mots = std::find(mots.begin(), mots.end(), '-' + param->name);
            if (it_mots != mots.end()) {
                const auto fst_it_mots = it_mots;
                if (param->is_bool()) {
                    param->set("true");
                }
                else {
                    if (++it_mots == mots.end()) {
                        throw ParseException("Parametre optionnel manquant.");
                    }
                    if (!param->set(*it_mots)) {
                        throw ParseException("Parametre optionnel invalide.");
                    }
                    mots.erase(it_mots);
                }
                mots.erase(fst_it_mots);
            }
        }

        // Gestion (et suppression) des arguments requis (les seuls restants)
        for (BaseParam* param : cmd.params_req) {
            if (mots.empty()) {
                // Cas ou le parametre a une valeur par defaut
                if (param->set_default())
                    continue; // Tous les suivants doivent �tre mis a leur valeur par defaut
                else
                    throw ParseException("Parametre requis manquant.");
            }
            if (!param->convert(mots.front())) {
                throw ParseException("Parametre requis invalide.");
            }
            mots.pop_front();
        }

        if (!mots.empty())
            throw ParseException("Parametre additionnel non reconnu.");

        // Execution
        cmd.command->execute(state);
    }
    else {
        throw ParseException("Commande non trouvee.");
    }

}

Terminal::Terminal() :
    commandes(), state()
{
    state.alive = true;
    state.path = "C:\\";
    this->addCommand(new ExitCommand());
    this->addCommand(new HelpCommand(this));
}


void Terminal::addCommand(Command* command) {
    auto insertPair = commandes.insert({ command->name(), CmdFormat(command) });
    if (!insertPair.second)
        throw std::runtime_error("Commande deja rajoute !");
    command->configure(insertPair.first->second);
}



void HelpCommand::execute(TerminalState& state)
{
    if (cmd_name.value == "") {
        *state.fluxOut << "Liste de commandes disponibles :\n";
        for (auto pair : commands) {
            *state.fluxOut << " - " << pair.first << " : " << pair.second.getCommand()->description() << "\n";
        }
        *state.fluxOut << std::endl;
    }
    else {
        auto searchedCmd = commands.find(cmd_name.value);
        if (searchedCmd == std::end(commands))
            throw ParseException("Pas de telle commande !");

        *state.fluxOut << searchedCmd->second.getCommand()->description();
        *state.fluxOut << std::endl;
        *state.fluxOut << "\nListe des parametres :\n";
        for (auto req : searchedCmd->second.getReqs()) {
            *state.fluxOut << " - " << req->name << " : " << req->description << "\n";
        }
        *state.fluxOut << std::endl;
        *state.fluxOut << "\nListe des options :\n";
        for (auto opt : searchedCmd->second.getOpts()) {
            *state.fluxOut << " - " << opt->name << " : " << opt->description << "\n";
        }
        *state.fluxOut << std::endl;
    }
}