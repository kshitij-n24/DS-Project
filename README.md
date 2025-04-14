# Distributed Group Based File Sharing System

I build a group based file sharing system where users can share, download files from the group they belong to. Download happens parallelly with multiple pieces from multiple peers, parallel downloading and custom piece selection algorithm.

## Architecture Overview

#### Tracker

- Maintain information of clients, groups they belongs to with their files to assist them for the communication between peers

- Tracker must be online 24/7

#### Client

- Create an account and register with the Tracker using userid and password to be part of the network

- Login everytime using user credentials whenever it want to operate within network

- Can create any number of groups (groupid should be unique accross the network)

- The client which create the group, by default should become owner of that group.

- If owner leaves the group, the member which joined the group just after admin will be a new admin of a group

- Client needs to be a part/member of a group from which it wants to download the file

- Fetch list of all groups in server

- Request to join a group

- Leave group

- List/Accept group joining requests (If owner)

- Share file across group: Share the filename and SHA1 hash of the completefile as well as piecewise SHA1 with the tracker

- fetch list of all shareble files in a group

- Download File \[Core Part\]

    - Retrieve peer information from tracker for the file you want to download from the group

    - After fetching the peer info from the tracker, client will communicate with peers about the portions of the file they contain and hence accordingly decide which part of file to take from which peer

    - Download file from multiple peers (different pieces of file from different peers - piece selection algorithm) simultaneously and all the files which client downloads will be shareable to other users in the same group

    - pieces are downloaded from more than 1 peers (if available)

    - Ensuring file integrity from SHA1 comparison

    - Users are able to download files concurrently in their respective sessions.

- Show downloads

- Stop sharing file

- Stop sharing all files temporarily (Logout)

- Whenever client logins, all previously shared files before logout should automatically be on sharing mode

# Build
Clone this project locally  
Run make command from both of the directory
```bash
cd /path/to/tracker
make
cd /path/to/client
make
```
it will create `./tracker` and `./client` executables respectively

# Commands

## Tracker

```bash
cd /path/to/tracker
make
```

### Run Tracker:

```bash
./tracker <trackerinfo.txt> 1
```

### Close Tracker:

```bash
quit
```

## Client

```bash
cd /path/to/client
make
```

### Run Client:

```bash
./client <IP>:<PORT> <trackerinfo.txt> 1
```

### Create Account:

```bash
create_user <user-name> <password>
```
### Login:

```bash
login <user-name> <password>
```
### Create Group:

```bash
create_group <group-name>
```
### Join Group:

```bash
join_group <group-name>
```
### Leave Group:

```bash
leave_group <group-name>
```
### List Pending Join (Admin Only):

```bash
list_requests <group-name>
```
### Accept Group Joining Request (Admin Only):

```bash 
accept_request <group-name> <user-name>
```
### List Groups:

```bash
list_groups
```
### List Shared Files in the Group:

```bash
list_files <group-name>
```
### Upload File:

```bash
upload_file <file-path> <group-name>
```
### Download File:

```bash
download_file <group-name> <file-name> <destination-path>
```
### Logout:

```bash
logout
```
### Show Downloads:

```bash
show_downloads
```
### Stop Sharing:

```bash
stop_sharing <group-name> <file-name>
```



