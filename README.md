compile server: g++ -std=c++17 -o tracker tracker.cpp -pthread
compile client: g++ -std=c++17 -o client peer.cpp -pthread

wywolywanie servera: ./tracker tracker_info.txt
wywolywanie klienta:  ./client ::1 tracker_info.txt

zrobione jest automatyczne przypisywanie portów i ipv6.

#### Functionality for client

* Create User Account: `create_user <user_id> <passwd>`
* Login: `login  <user_id> <passwd>`
* Create Group: `create_group <group_id>`
* Join Group: `join_group <group_id>`
* Leave Group: `leave_group <group_id>`
* List pending join: `list_requests <group_id>`
* Accept Group Joining Request: `accept_request <group_id> <user_id>`
* List All Group In Network: `list_groups`
* List All sharable Files In Group: `list_files <group_id>`
* Upload File: `upload_file <file_path> <group_id>`
* Download File: `download_file <group_id> <file_name> <destination_path>`
* Logout: `logout`
* Show_downloads: `show_downloads`
  * Output format:
    * [D] [grp_id] filename
    * [C] [grp_id] filename
    * D(Downloading), C(Complete)
* Stop sharing: `stop_share <group_id> <file_name>`
