#include "Descriptor.hpp"

#include "comm.hpp"
#include "string_utils.hpp"

// Up to 5 characters
const char *short_name_of(DescriptorState state) {
    switch (state) {
    case DescriptorState::Playing: return "Play";
    case DescriptorState::GetName: return "Name";
    case DescriptorState::GetOldPassword: return "OldPw";
    case DescriptorState::ConfirmNewName: return "CnfNm";
    case DescriptorState::GetNewPassword: return "NewPw";
    case DescriptorState::ConfirmNewPassword: return "CnfNp";
    case DescriptorState::GetNewRace: return "GetNR";
    case DescriptorState::GetNewSex: return "GetNS";
    case DescriptorState::GetNewClass: return "GetNA";
    case DescriptorState::GetAlignment: return "GetAl";
    case DescriptorState::DefaultChoice: return "DefCh";
    case DescriptorState::GenGroups: return "GenGr";
    case DescriptorState::ReadIMotd: return "Imotd";
    case DescriptorState::ReadMotd: return "Motd";
    case DescriptorState::BreakConnect: return "BrkCn";
    case DescriptorState::GetAnsi: return "Ansi";
    case DescriptorState::CircumventPassword: return "CPass";
    case DescriptorState::Disconnecting: return "Disc";
    case DescriptorState::DisconnectingNp: return "DiscN";
    }
    return "<UNK>";
}

const char *name_of(DescriptorState state) {
    switch (state) {
    case DescriptorState::Playing: return "Playing";
    case DescriptorState::GetName: return "GetName";
    case DescriptorState::GetOldPassword: return "GetOldPassword";
    case DescriptorState::ConfirmNewName: return "ConfirmNewName";
    case DescriptorState::GetNewPassword: return "GetNewPassword";
    case DescriptorState::ConfirmNewPassword: return "ConfirmNewPassword";
    case DescriptorState::GetNewRace: return "GetNewRace";
    case DescriptorState::GetNewSex: return "GetNewSex";
    case DescriptorState::GetNewClass: return "GetNewClass";
    case DescriptorState::GetAlignment: return "GetAlignment";
    case DescriptorState::DefaultChoice: return "DefaultChoice";
    case DescriptorState::GenGroups: return "GenGroups";
    case DescriptorState::ReadIMotd: return "ReadIMotd";
    case DescriptorState::ReadMotd: return "ReadMotd";
    case DescriptorState::BreakConnect: return "BreakConnect";
    case DescriptorState::GetAnsi: return "GetAnsi";
    case DescriptorState::CircumventPassword: return "CircumventPassword";
    case DescriptorState::Disconnecting: return "Disconnecting";
    case DescriptorState::DisconnectingNp: return "DisconnectingNp";
    }
    return "Unknown";
}

Descriptor::Descriptor(uint32_t descriptor) {
    this->descriptor = descriptor;
    outbuf = (char *)alloc_mem(outsize);
    logintime = str_dup((char *)ctime(&current_time));
    host = str_dup("(unknown)");
}

Descriptor::~Descriptor() {
    free_string(host);
    /* RT socket leak fix -- I hope */
    free_mem(outbuf, outsize);
    /*    free_string(dclose->showstr_head); */
}

std::optional<std::string> Descriptor::pop_raw() {
    if (pending_commands_.empty())
        return std::nullopt;
    auto result = std::move(pending_commands_.front());
    pending_commands_.pop_front();
    return result;
}

std::optional<std::string> Descriptor::pop_incomm() {
    auto maybe_next = pop_raw();
    if (!maybe_next)
        return {};

    // Handle any backspace characters, newlines, or non-printing characters.
    auto next_line = sanitise_input(*maybe_next);

    if (next_line.size() > MAX_INPUT_LENGTH - 1) {
        write("Line too long.\n\r");
        return {};
    }

    // Do '!' substitution.
    if (next_line[0] == '!')
        return last_command_;
    else {
        last_command_ = next_line;
        return next_line;
    }
}

bool Descriptor::write(std::string_view text) const {
    Packet p;
    p.type = PACKET_MESSAGE;
    p.channel = descriptor;

    while (!text.empty()) {
        p.nExtra = std::min<uint32_t>(text.length(), PACKET_MAX_PAYLOAD_SIZE);
        if (!SendPacket(&p, text.data()))
            return false;
        text = text.substr(p.nExtra);
    }
    return true;
}