sudo apt update
sudo apt install zsh tmux git curl -y
sudo apt install nghttp2 libnghttp2-dev libssl-dev -y
sudo apt install build-essential g++ gcc cmake xclip -y

sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)"

git clone https://github.com/neovim/neovim.git ~/code
cd ~/code/neovim
make CMAKE_BUILD_TYPE=RelWithDebInfo
sudo make install

git clone https://github.com/LazyVim/starter ~/.config/nvim

git clone https://github.com/zsh-users/zsh-autosuggestions $ZSH_CUSTOM/plugins/zsh-autosuggestions

git clone https://github.com/zsh-users/zsh-syntax-highlighting.git $ZSH_CUSTOM/plugins/zsh-syntax-highlighting

git clone https://github.com/tmux-plugins/tpm ~/.tmux/plugins/tpm

DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cp "$DIR/tmux.conf" ~/.tmux.conf
