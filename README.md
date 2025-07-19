## **Site tracker** 
Using https libraries to track similarities between sites inputted

## **Features:**
- Uses libCurl and OpenSSL to target TLS certificates, response headers, status codes server info, status codes and more
- Adds information into a matrix storage 
- (in progress) matrix will sort and group sites based on user inputted similarity checks
- Uses concurrency to optimize for speed 

##  **Key Files & Structure**  
- **`main.cpp`**  Used to concurrently grab site information 

##  **Setup Guide**  
### Clone the Repository  
```bash  
git clone https://github.com/siddharthr2004/SiteSimilarityTracker.git  
cd SiteSimilarityTracker  
```  

###  **Install Dependencies  
```bash  
pip install -r requirements.txt  
npm install  
```  

### 4️⃣ Start the program  
```bash  
run 'make'
```  
