#ifndef TERMINAL_HPP
#define TERMINAL_HPP


#include<string>
#include<iostream>
#include<unordered_map>
#include<list>
#include<vector>
#include<exception>
#include<algorithm>
#include<fstream>
#include<sstream>




class ParseException : public std::exception {
public:
    explicit ParseException(const std::string& message) : msg(message) {}
    const char* what() const noexcept override { return msg.c_str(); }
private:
    std::string msg;
};


struct TerminalState {
    bool alive;
    std::string path;
    std::istream* fluxIn = &std::cin;
    std::ostream* fluxOut = &std::cout;
};

class BaseParam {
public:
    BaseParam(std::string name, std::string description) : name(name), description(description) {};

    bool set(const std::string& value) { isSet = true; return convert(value); }
    virtual void reset() { isSet = false; }
    operator bool() { return isSet; }

    virtual bool is_bool() const { return false; };

    std::string name;
    std::string description;

protected:
    virtual bool convert(const std::string& value) = 0;
    virtual bool set_default() = 0;

    friend class Terminal;

protected:
    bool isSet = false;

};

template <typename T>
class TypedParam : public BaseParam {
public:
    TypedParam(std::string name, std::string description) :
        BaseParam(name, description),
        value(), m_can_default(false), m_def_value()
    {
    }

    void giveDefValue(T def_value) {
        m_def_value = def_value;
        m_can_default = true;
    }

    void reset() override { BaseParam::reset(); set_default(); }

    T value;

protected:
    bool set_default() override { value = m_def_value; return m_can_default; }

private:
    bool m_can_default;
    T m_def_value;

};

class Command;

class CmdFormat {
public:
    CmdFormat(Command* command) : command(command) {}

    CmdFormat& addReq(BaseParam* param) {
        params_req.push_back(param);
        return *this;
    }
    CmdFormat& addOpt(BaseParam* param) {
        params_opt.push_back(param);
        return *this;
    }

    const std::vector<BaseParam*>& getReqs() const { return params_req; }
    const std::vector<BaseParam*>& getOpts() const { return params_opt; }
    const Command* getCommand() const { return command; }

private:
    friend class Terminal;

    std::vector<BaseParam*> params_req;
    std::vector<BaseParam*> params_opt;
    Command* command;
};


class Command {
public:
    virtual ~Command() = default;

    virtual const char* name() const = 0;
    virtual const char* description() const = 0;
    virtual void execute(TerminalState& state) = 0;

protected:
    virtual void configure(CmdFormat& format) = 0;

    friend class Terminal;
};



class Terminal {
public:
    TerminalState state;

    Terminal();

    void addCommand(Command* command);
    const std::unordered_map<std::string, CmdFormat>& getCommands() const {
        return commandes;
    }

    void run();
    void kill() { state.alive = false; }

private:
    // Parse une ligne de commande et retourne un pointeur vers la Command correspondante, ou nullptr si non trouv�e
    void parse(const std::string& ligne, TerminalState& state);


private:
    std::unordered_map<std::string, CmdFormat> commandes;

};





#pragma region TypedParam



class IntParam : public TypedParam<int> {
public:
    IntParam(std::string name, std::string description)
        : TypedParam(std::move(name), std::move(description))
    {
    }

protected:
    bool convert(const std::string& strval) override {
        std::istringstream iss(strval);
        int temp;
        if (iss >> temp) {
            value = temp;
            return true;
        }
        return false;
    }


};

class BoolParam : public TypedParam<bool> {
public:
    BoolParam(std::string name, std::string description)
        : TypedParam(std::move(name), std::move(description))
    {
        value = false;
    }

    bool is_bool() const override {
        return true;
    }

protected:
    bool convert(const std::string& strval) override {
        // Transform in lowercase for easier comparison
        std::string val = strval;
        std::transform(val.begin(), val.end(), val.begin(), ::tolower);

        if (val == "true" || val == "1" || val == "yes" || val == "y") {
            value = true;
            return true;
        }
        else if (val == "false" || val == "0" || val == "no" || val == "n") {
            value = false;
            return true;
        }
        return false;
    }

};

class DoubleParam : public TypedParam<double> {
public:
    DoubleParam(std::string name, std::string description)
        : TypedParam(std::move(name), std::move(description))
    {
    }

    bool convert(const std::string& strval) override {
        std::istringstream iss(strval);
        double temp;
        if (iss >> temp) {
            value = temp;
            return true;
        }
        return false;
    }

};

class StringParam : public TypedParam<std::string> {
public:
    StringParam(std::string name, std::string description)
        : TypedParam(std::move(name), std::move(description))
    {
    }

protected:
    bool convert(const std::string& strval) override {
        value = strval;
        return true;
    }

};

class FilePathParam : public StringParam {
public:
    FilePathParam(std::string name, std::string description, std::ios_base::openmode mode = std::ios::in)
        : StringParam(std::move(name), std::move(description)),
        open_mode(mode), file_stream(std::make_unique<std::fstream>()) {
    }

protected:
    // Tente d'ouvrir le fichier � la conversion, value = nom du fichier
    bool convert(const std::string& strval) override {
        file_stream.reset(new std::fstream); // Ferme le fichier pr�c�dent si besoin
        file_stream->open(strval, open_mode);
        if (file_stream->is_open()) {
            value = strval;
            return true;
        }
        file_stream.reset();
        value.clear();
        return false;
    }


public:
    // Fournit un acces au flux de fichier ouvert (nullptr si erreur)
    std::fstream* stream() const { return file_stream.get(); }

private:
    std::ios_base::openmode open_mode;
    std::unique_ptr<std::fstream> file_stream;

};


#pragma endregion



#pragma region DefCommand


class ExitCommand : public Command {
public:
    const char* name() const override { return "exit"; }
    const char* description() const override { return "tue le terminal"; }

    void execute(TerminalState& state) override { state.alive = false; }

protected:
    void configure(CmdFormat& format) override {}

};

class HelpCommand : public Command {
public:
    HelpCommand(const Terminal* term) :
        commands(term->getCommands()),
        cmd_name("cmd_name", "nom de la commande sur laquelle vous voulez connaitre des informations (opt)")
    {
    }

    const char* name() const override { return "help"; }
    const char* description() const override { return "affiche un message d'aide"; }

    void execute(TerminalState& state) override;

protected:
    void configure(CmdFormat& format) override {
        cmd_name.giveDefValue("");
        format.addReq(&cmd_name);
    }

    StringParam cmd_name;
    const std::unordered_map<std::string, CmdFormat>& commands;
};


#pragma endregion

#endif // TERMINAL_HPP 