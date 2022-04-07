#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include <iterator>
#include <iomanip>
#include <windows.h>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <conio.h>
#include <sstream>
#include <cstdio>
#include "oleanderstemminglibrary/include/olestem/stemming/english_stem.h"
#include <vector>
#include <numeric>
#include <functional>
#include <set>

using namespace std;

int main()
{
    // Getting and storing stopwords from the stopwords file
    string stopwordsfile="Stopword-List.txt";
    ifstream fr;
    string stop[26],stopoutput;

    int i=0,j=0,k=0,flag=0;
    fr.open(stopwordsfile,ios::in);
    while(1)
    {
        fr>>stopoutput;
        stop[i]=stopoutput;
        i++;
        if(i>25)
        {
            break;
        }
    }

    // Getting data from files within Abstracts folder
    string filestr="Abstracts\\";
    string out="",temp="",t="",convert,altconvert,altreconv,reconv;
    vector <string> storage,postorage;
    
    // Creating multimap for storing data in (key,value) pairs
    multimap <string,int> dict;

    // Defining two custom datatypes, a vector of tuples with (string,int,int) and another vector of tuples with (int,int)
    // for proximity queries (Storing words as (word,doc,wordpos))
    typedef vector<tuple<string,int,int>> posdict;
    typedef vector<tuple<int,int>> positional;
    posdict pos,tempos;

    //Creating a stemmer using the custom library
    stemming::english_stem<> StemEnglish;
    wstring altword,word;


    // Main loop to getting data from all files
    for(k=1;k<=448;k++)
    {
        filestr+=to_string(k);
        filestr+=".txt";
        ifstream f;
        f.open(filestr,ios::in);
        while(!f.eof())
        {
            flag=0;

            // Getting words line by line
            getline(f,out);
            
            if(out!="")
            {
                for(j=0;j<out.length();j++)
                {
                    // Converting characters in line to lowercase if their characters are uppercase
                    if(isupper(out[j]))
                    {
                        out[j]=tolower(out[j]);
                    }
                }
                for(i=0;i<out.length();i++)
                { 
                    // C++ does have a built in tokenizor, but due to problems with it I created my own tokenizor of sorts
                    // That seperates on the basis of most special characters
                    if (out[i+1]==0)
                    {
                        t=out[i];
                        flag=1;
                    }
                    if(  out[i]!=' ' && out[i]!='-' && out[i]!='(' && out[i]!=')' && out[i]!='/' && out[i]!=',' && out[i]!='.' && out[i+1]!=0 )
                    { 
                        temp+=out[i];
                    }
                    else if(  out[i]==' ' || out[i]=='-' || out[i]=='(' || out[i]==')' || out[i]=='/' || out[i]=='.' || out[i]==',' || out[i+1]==0)
                    {
                        // Loop to check if any of the words are stopwords
                        for(j=0;j<26;j++)
                        {
                            if(stop[j]==temp)
                            {
                                // If they are, they are stored because stopwords are considered for proximity queries
                                postorage.push_back(temp);
                                temp="";
                            }
                        }
                        if(temp!="")
                        {
                            if(flag==1 && (isalpha(t[0]) || isdigit(t[0]) ))
                            {
                                temp+=t;
                                t.clear();
                                flag=0;
                            }

                            // If the words are alphabets and not special characters, they are stored in both vectors below
                            // The other vector is used primarly for boolean queries, whereas the previous one is used for proximity queries
                            postorage.push_back(temp);
                            storage.push_back(temp);
                        }   
                        temp="";
                    }
                }      
            }
            // Removal of all blank/white spaces just to be certain
            for(j=0;j<storage.size();j++)
            {
                if(storage.at(j)==" " || storage.at(j)=="")
                {
                    storage.erase(storage.begin()+j);
                }
            }
            // Same thing for the other vector
            for(j=0;j<postorage.size();j++)
            {
                if(postorage.at(j)==" " || postorage.at(j)=="")
                {
                    postorage.erase(postorage.begin()+j);
                }
            }
               
        }
        // Stemming the words within the vectors
        for(j=0;j<storage.size();j++)
        {
            // Due to the custom library using only wstrings, the conversion of the string inside the vectors to wstrings was required
            convert+=storage.at(j);
            wchar_t* UnicodeTextBuffer = new wchar_t[convert.length()+1];
            wmemset(UnicodeTextBuffer, 0, convert.length()+1);
            mbstowcs(UnicodeTextBuffer, convert.c_str(), convert.length());
            word = UnicodeTextBuffer;

            // The converted wstring was stemmed using the libraries function
            StemEnglish(word);
            convert.clear();reconv.clear();

            // Conversion of wstring back to string to be able to store it in the multimap, and inserting it in the multimap
            string reconv(word.begin(),word.end());

            // Stored in the form of (word,docID)
            dict.insert(pair<string,int>(reconv.c_str(),k));
        }  
        // Same thing for the other vector
        for(j=0;j<postorage.size();j++)
        {
            // Similar conversion performed here
            altconvert+=postorage.at(j);
            wchar_t* UnicodeTextBuffer = new wchar_t[altconvert.length()+1];
            wmemset(UnicodeTextBuffer, 0, altconvert.length()+1);
            mbstowcs(UnicodeTextBuffer, altconvert.c_str(), altconvert.length());
            altword = UnicodeTextBuffer;

            // The converted wstring was stemmed using the libraries function
            StemEnglish(altword);
            altconvert.clear();
            altreconv.clear();

            // Conversion of wstring back to string to be able to store it in the vector of tuples, and inserting it in the vector of tuples
            string altreconv(altword.begin(),altword.end());

            // Stored in the form of (word,docID,wordPos)
            pos.push_back(tuple<string,int,int>(altreconv.c_str(),k,j));
        }  
        postorage.clear();
        storage.clear();
        filestr.clear();
        filestr="Abstracts\\";
    }

    // Viewing key-value pairs
    multimap<string,int>::iterator itr;
    posdict::const_iterator iter;

    // String parsing
    string query="";

    // Typing any of these words will end the program
    while(query!="quit" || query!="end" || query!="exit" )
    {
        cout<<"Enter query: ";
        getline(cin,query);

        // The query, as mentioned in the readme, can only ever contain a maximum of 7 words 
        string t[7]="";
        int querywords=0;
        int flag=0;

        // Main loop to split the words in the query seperately, after converting them to lowercase
        // It is assumed that the user is capable and understands the system well enough to be able to write a correct query
        // The queries are of the form=

        // {Boolean/Complex Boolean}

        // 1. w1 
        // 2. not w1
        // 3. w1 and/or w2
        // 4. w1 and/or not w2
        // 5. w1 and/or w2 and/or w3
        // 6. w1 and/or not w2 and/or w3
        //     w1 and/or w2 and/or not w3
        // 7. w1 and/or not w2 and/or not w3

        // {Proximity Queries}

        // 8. w1 w2 /n (n being the distance between w1 and w2) 

        // {Phrase Queries}

        // 9. w1 w2 w3

        for(int i=0;i<query.length();i++)
        {
            query[i]=tolower(query[i]);
            if(flag==0 && query[i]!=' ')
            {
                t[0]+=query[i];
            }
            else if(flag==1 && query[i]!=' ')
            {
                t[1]+=query[i];
            }
            else if(flag==2 && query[i]!=' ')
            {
                t[2]+=query[i];
            }
            else if(flag==3 && query[i]!=' ')
            {
                t[3]+=query[i];
            }
            else if(flag==4 && query[i]!=' ')
            {
                t[4]+=query[i];
            }
            else if(flag==5 && query[i]!=' ')
            {
                t[5]+=query[i];
            }
            else if(flag==6 && query[i]!=' ')
            {
                t[6]+=query[i];
            }

            if(query[i]==' ' && flag==0)
            {
                flag=1;
            }
            else if(query[i]==' ' && flag==1)
            {
                flag=2;
            }
            else if(query[i]==' ' && flag==2)
            {
                flag=3;
            }
            else if(query[i]==' ' && flag==3)
            {
                flag=4;
            }
            else if(query[i]==' ' && flag==4)
            {
                flag=5;
            }
            else if(query[i]==' ' && flag==5)
            {
                flag=6;
            }
        }
        // Counting the number of words in a query, for custom cases discussed below
        for(i=0;i<7;i++)
        {
            if(t[i]!="")
            {
                querywords+=1;
            }
        }

        // Parsing and stemming the query as well, to match the input with the output
        // Conversion is entirely similar as above, only change is the replacement of a string with an array of strings
        wstring convterms[7];
        string terms[7];
        for(i=0;i<querywords;i++)
        {
            wchar_t* UnicodeTextBuffer = new wchar_t[t[i].length()+1];
            wmemset(UnicodeTextBuffer, 0, t[i].length()+1);
            mbstowcs(UnicodeTextBuffer, t[i].c_str(), t[i].length());
            convterms[i] = UnicodeTextBuffer;
            StemEnglish(convterms[i]);
            terms[i]=string(convterms[i].begin(),convterms[i].end());
        }
 
        // Creation of several vectors, which are used to perform set operations in the STL library
        // These vectors store the individual results of parts of the query, and perform set operations to get the final answer
        vector <int> occ1,occ2,occ3,terocc,alphaocc,betaocc,gammaocc,totalocc,globalocc;

        // Creation of a global query, so (NOT w1) type queries can be entertained using the set difference concept
        for(i=1;i<=448;i++)
        {
            globalocc.push_back(i);
        }

        // Various flags used to make sure the same word in the same doc isnt stored twice in the same vector
        // i.e. w1 w1 in doc 1 will only lead to w1 being stored once, not twice
        int check1=0,check2=0,check3=0;

        // The count vector ensures that the results are only printed as long as there is SOME data within the result vector
        // This check is applied near the very end
        vector <int> count;

         // Creation of an iterator type data structure for the multimap
       
        typedef multimap <string, int> ::iterator MAPIterator;

        // Condition if a query is of length 1, i.e. it is the form
        // w1
        // 0
        if(querywords==1)
        {
            // Return the range of documents where the term w1 occurs
            pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);
            count.push_back(distance(result1.first, result1.second));

            // Using the multimap to iterate over the returned range
            for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
            {
                // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                // As discussed, this is done using the varios flags we discussed before

                // occ1.at(i) returns the term inside the vector occ1, which is used to store the results
                // Will not run initially, as occ1 has no size, only on the second iteration does it have some size
                for(i=0;i<occ1.size();i++)
                {
                    // Ensures that only a unique docID is stored in the vector
                    if(occ1.at(i)==it1->second)
                    {
                        check1=1;
                    }
                }
                if(check1==0)
                {
                    // Stored the very first resulting docID in the vector
                    occ1.push_back(it1->second);
                }

                // Resetting the flag
                check1=0;
            }

            // Stores all the docIDs in a vector of totalocc (total occurrences)
            // All results in all future conditions will only ever be stored in totalocc vector
            for(i=0;i<occ1.size();i++)
            {
                totalocc.push_back(occ1.at(i));
            }
        }

        // Condition if a query is of length 2, i.e. it is the form
        // not w1
        // 0   1
        else if(querywords==2 && terms[0]=="not")
        {
            // Entirely similar as above
            pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[1]);
            for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
            {
                // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                for(i=0;i<occ1.size();i++)
                {
                    if(occ1.at(i)==it1->second)
                    {
                        check1=1;
                    }
                }
                if(check1==0)
                {
                    occ1.push_back(it1->second);
                }
                check1=0;
            }
            
            // Subtracting the resultant documents from the global vector containing all document IDs
            set_difference(globalocc.begin(),globalocc.end(),occ1.begin(),occ1.end(),back_inserter(totalocc));
            count.clear();

            //Storing the size of the resultant vector in count
            count.push_back(totalocc.size());
        }


        // Condition if a query is of length 3, i.e. it is the form
        // w1 and/or w2
        // 0     1    2
        // Additionally, the third term cannot start with a /, since this condition is for a proximity query
        else if( (terms[1] == "and" || terms[1]=="or") && querywords==3 && terms[2][0]!='/')
        {
            // Entirely similar, except this time two terms are stored, one here, and one below
            // The resultant vectors of both terms are either used to perform intersection or union
            // depending on and/or, and stored in the final vector totalocc

            pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);

            for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
            {
                // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                for(i=0;i<occ1.size();i++)
                {
                    if(occ1.at(i)==it1->second)
                    {
                        check1=1;
                    }
                }
                if(check1==0)
                {
                    occ1.push_back(it1->second);
                }
                check1=0;
            }
            
            pair <MAPIterator, MAPIterator> result2 = dict.equal_range(terms[2]);

            for (MAPIterator it2 = result2.first; it2 != result2.second; it2++)
            {
                // Repeating for second term
                for(i=0;i<occ2.size();i++)
                {
                    if(occ2.at(i)==it2->second)
                    {
                        check2=1;
                    }
                }
                if(check2==0)
                {
                    occ2.push_back(it2->second);
                }
                check2=0;
            }

            // Store both vector results
            if (terms[1]=="or")
            {
                for(i=0;i<occ1.size();i++)
                {
                    totalocc.push_back(occ1.at(i));    
                }
                for(i=0;i<occ2.size();i++)
                {
                    totalocc.push_back(occ2.at(i)); 
                }
            }

            // Store only vector results where both w1 and w2 occur
            else if (terms[1]=="and")
            {
                set_intersection(occ1.begin(),occ1.end(),occ2.begin(),occ2.end(),back_inserter(totalocc));
            }
            count.push_back(totalocc.size());
        }
        
        // Condition if a query is of length 3, i.e. it is the form
        // w1 w2 /n
        // 0  1   2
        // This condition is for a proximity query, since the first character of the third term will be a '/'
        else if(querywords==3 && terms[2][0]=='/')
        {
            // Storing the number in a string specified after the '/'
            string num;
            for(int it=0;it<terms[2].length();it++)
            {
                if(isdigit(terms[2][it]))
                {
                    num+=terms[2][it];
                }
            }
            int actualnum;

            // Converting the string to a number
            actualnum=stoi(num);

            // First, we must get the documents in which both terms occur
            pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);

            for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
            {
                // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                for(i=0;i<occ1.size();i++)
                {
                    if(occ1.at(i)==it1->second)
                    {
                        check1=1;
                    }
                }
                if(check1==0)
                {
                    occ1.push_back(it1->second);
                }
                check1=0;
            }
            
            pair <MAPIterator, MAPIterator> result2 = dict.equal_range(terms[1]);

            for (MAPIterator it2 = result2.first; it2 != result2.second; it2++)
            {
                // Repeating for second term
                for(i=0;i<occ2.size();i++)
                {
                    if(occ2.at(i)==it2->second)
                    {
                        check2=1;
                    }
                }
                if(check2==0)
                {
                    occ2.push_back(it2->second);
                }
                check2=0;
            }

            positional p1,p2;

            // Performing set intersection to only get relevant documents, and storing in a vector
            set_intersection(occ1.begin(),occ1.end(),occ2.begin(),occ2.end(),back_inserter(alphaocc));

            posdict::const_iterator ir;

            // Iterating for every term in alphaocc
            for(j=0;j<alphaocc.size();j++)
            {
                // Iterating and checking all positions of those documents using the vector of tuples we created
                for(ir=pos.begin(); ir!=pos.end(); ir++) 
                {
                    // Checking if docID of the alphaocc vector and in our vector of tuples [ (string,int,int) variant] is same, and
                    // checking if the term stored inside our vector of tuples and in the user entered string is the same

                    // The referencing here is difficult to understand, to make it easier, it works like
                    // get<0> = term, get<1> == docID, get<2> == termPos
                    if(get<1>(*ir)==alphaocc.at(j) && get<0>(*ir)==terms[0])
                    {
                        // Storing it in a vector of tuple [ (int,int) variant] 
                        p1.push_back(tuple<int,int>(get<1>(*ir),get<2>(*ir)));
                    }
                    else if(get<1>(*ir)==alphaocc.at(j) && get<0>(*ir)==terms[1])
                    {
                        // Storing it in a vector of tuple [ (int,int) variant] 
                        p2.push_back(tuple<int,int>(get<1>(*ir),get<2>(*ir)));
                    }
                }
            }

            // Creating two iterators to iterate over the results we stored earlier in the vector of tuple [ (int,int) variant] 
            positional::const_iterator evenir,oddir;

            int tempnum=0;

            // Our answers are now of the form (docID,termPosition)
            for(evenir=p1.begin();evenir!=p1.end(); evenir++)
            {
                for(oddir=p2.begin();oddir!=p2.end(); oddir++)
                {
                    // In this nested loop, we are checking the positions stored in the tuples
                    // What we are checking is if the difference between two positions is equal to the 'n' (or within it) the user specified

                    // To re-iterate for clarity using an example
                    // If the user enters a query
                    // feature track /5
                    // It will first get all documents where feature and track occur
                    // Then store this data in a vector of tuples of the form
                    // (word,docID,wordPos)
                    // Now it will compare their wordPos, and check if the difference between them is within 5 words
                    // Such as
                    // (feature,13,5)
                    // (track,13,11)
                    // Since the difference is within 5 words and the docID is the same, it'll store it in the final vector
                    // However,
                    // (feature,13,5)
                    // (track,212,8)
                    // Will not store the resultant doc, since the docID is different
                    // The referencing here can be difficult to understand, basically it is
                    // get<0> = docID, get<1> is termPos

                    tempnum=abs((get<1>(*oddir) - get<1>(*evenir))) - 1;
                    if( (tempnum<=actualnum) && (get<0>(*oddir)==get<0>(*evenir)) )
                    {
                        totalocc.push_back(get<0>(*oddir));
                    }
                }   
            }
            // cout<<endl<<tempnum;
            count.push_back(totalocc.size());
            cout<<endl;

        }

        // Condition if a query is of length 3, i.e. it is the form
        // w1 w2 w3
        // 0  1  2
        // This is the phrase query condition
        else if( (terms[1] != "and" && terms[1]!="or") && querywords==3 && terms[2][0]!='/')
        {
            pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);

            for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
            {
                // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                for(i=0;i<occ1.size();i++)
                {
                    if(occ1.at(i)==it1->second)
                    {
                        check1=1;
                    }
                }
                if(check1==0)
                {
                    occ1.push_back(it1->second);
                }
                check1=0;
            }

            pair <MAPIterator, MAPIterator> result3 = dict.equal_range(terms[2]);

            for (MAPIterator it3 = result3.first; it3 != result3.second; it3++)
            {
                // Repeating for third term
                for(i=0;i<occ3.size();i++)
                {
                    if(occ3.at(i)==it3->second)
                    {
                        check3=1;
                    }
                }
                if(check3==0)
                {
                    occ3.push_back(it3->second);
                }
                check3=0;
            }

            

            positional p1,p2,p3;

            set_intersection(occ1.begin(),occ1.end(),occ3.begin(),occ3.end(),back_inserter(alphaocc));

            // The above code will return all documents with both w1 and w3, similarly discussed before

            posdict::const_iterator ir;

            // These loops will store the positions of those words, similary as before
            for(j=0;j<alphaocc.size();j++)
            {
                for(ir=pos.begin(); ir!=pos.end(); ir++) 
                {
                    if(get<1>(*ir)==alphaocc.at(j) && get<0>(*ir)==terms[0])
                    {
                        p1.push_back(tuple<int,int>(get<1>(*ir),get<2>(*ir)));
                    }
                    else if(get<1>(*ir)==alphaocc.at(j) && get<0>(*ir)==terms[1])
                    {
                        p2.push_back(tuple<int,int>(get<1>(*ir),get<2>(*ir)));
                    }
                }
            }
            
            positional::const_iterator evenir,oddir;

            int tempnum=0;

            // These iterators will iterate over the given documents and check the positions, similary as before
            for(evenir=p1.begin();evenir!=p1.end(); evenir++)
            {
                for(oddir=p2.begin();oddir!=p2.end(); oddir++)
                {
                    // university of chicago
                    // 0          1     2

                    tempnum=abs((get<1>(*oddir) - get<1>(*evenir))) - 1;

                    // For this, the document was opened and the words within the query and the file were converted to a string
                    // with all punctuation and spaces removed
                    // The user given query was checked as a substring within the file string
                    // If there was a match, the document ID was stored in the final vector
                    if(tempnum==1 && get<0>(*oddir)==get<0>(*evenir) )
                    {
                        string tempfile="Abstracts\\";
                        tempfile+=to_string(get<0>(*oddir));
                        tempfile+=".txt";

                        string phrase;
                        phrase+=t[0]+t[1]+t[2];

                        string tempout="",permout="";
                        fstream flr;
                        int tflag=0,sflag=0,rflag=0;
                        flr.open(tempfile,ios::in);
                        while (!flr.eof())
                        {
                            // tempout==Children.
                            getline(flr,tempout);
                            for(int iterator=0;iterator<tempout.length();iterator++)
                            {
                                if(isdigit(tempout[iterator]) || isalpha(tempout[iterator]))
                                {
                                    tempout[iterator]=tolower(tempout[iterator]);
                                    permout+=tempout[iterator];
                                }
                            }
                            
                            // permout==children
                            // Storing the answer if the phrase substring exists within the file
                            if(strstr(permout.c_str(),phrase.c_str()))
                            {
                                totalocc.push_back(get<0>(*oddir));
                            }
                        }
                        
                    }
                }   
            }
            // cout<<endl<<tempnum;
            count.push_back(totalocc.size());
        }
        
        // Condition if a query is of length 4, i.e. it is the form
        // w1 and/or not w2
        // 0     1    2   3
        else if(querywords==4)
        {
            // This code will work entirely similarly to the previous conditions that werent phrase/proximity ones
            // Getting the documents where term 2 doesn't exist, and term1 does
            // And performing the relevant set operations between the gotten results

            pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);
            for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
            {
                // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                for(i=0;i<occ1.size();i++)
                {
                    if(occ1.at(i)==it1->second)
                    {
                        check1=1;
                    }
                }
                if(check1==0)
                {
                    occ1.push_back(it1->second);
                }
                check1=0;
            }
            
            pair <MAPIterator, MAPIterator> result2 ;
            result2 = dict.equal_range(terms[3]);

            for (MAPIterator it2 = result2.first; it2 != result2.second; it2++)
            {
                // Repeating for second term
                for(i=0;i<occ2.size();i++)
                {
                    if(occ2.at(i)==it2->second)
                    {
                        check2=1;
                    }
                }
                if(check2==0)
                {
                    occ2.push_back(it2->second);
                }
                check2=0;
            }
            // word1 or not word2
            if (terms[1]=="or")
            {
                set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(terocc));
                for(i=0;i<occ1.size();i++)
                {
                    totalocc.push_back(occ1.at(i));    
                }
                for(i=0;i<terocc.size();i++)
                {
                    totalocc.push_back(terocc.at(i)); 
                }
                count.push_back(totalocc.size());
            }
            // word1 and not word2
            else if (terms[1]=="and")
            {
                set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(terocc));
                set_intersection(occ1.begin(),occ1.end(),terocc.begin(),terocc.end(),back_inserter(totalocc));
                count.push_back(totalocc.size());
            }
            // It feels a little redundant at this point to re-explain the same code repeatedly, therefore I'll only try to comment on new things
        }

        // Condition if a query is of length 5, i.e. it is the form
        // w1 and/or  w2  and/or w3
        // 0    1     2     3     4
        else if(querywords==5)
        {
            pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);

            for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
            {
                // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                for(i=0;i<occ1.size();i++)
                {
                    if(occ1.at(i)==it1->second)
                    {
                        check1=1;
                    }
                }
                if(check1==0)
                {
                    occ1.push_back(it1->second);
                }
                check1=0;
            }
            
            pair <MAPIterator, MAPIterator> result2 = dict.equal_range(terms[2]);

            for (MAPIterator it2 = result2.first; it2 != result2.second; it2++)
            {
                // Repeating for second term
                for(i=0;i<occ2.size();i++)
                {
                    if(occ2.at(i)==it2->second)
                    {
                        check2=1;
                    }
                }
                if(check2==0)
                {
                    occ2.push_back(it2->second);
                }
                check2=0;
            }

            pair <MAPIterator, MAPIterator> result3 = dict.equal_range(terms[4]);

            for (MAPIterator it3 = result3.first; it3 != result3.second; it3++)
            {
                // Repeating for third term
                for(i=0;i<occ3.size();i++)
                {
                    if(occ3.at(i)==it3->second)
                    {
                        check3=1;
                    }
                }
                if(check3==0)
                {
                    occ3.push_back(it3->second);
                }
                check3=0;
            }

            // Performing relevant set operations based on conditions and storing within the final vector
            // word1 and word2 and word3
            if(terms[1]=="and" && terms[3]=="and")
            {
                set_intersection(occ1.begin(),occ1.end(),occ2.begin(),occ2.end(),back_inserter(terocc));
                set_intersection(occ3.begin(),occ3.end(),terocc.begin(),terocc.end(),back_inserter(totalocc));
            }

            // word1 or word2 and word3
            else if(terms[1]=="or" && terms[3]=="and")
            {
                set_intersection(occ2.begin(),occ2.end(),occ3.begin(),occ3.end(),back_inserter(terocc));
                for(i=0;i<terocc.size();i++)
                {
                    totalocc.push_back(terocc.at(i));    
                }
                for(i=0;i<occ1.size();i++)
                {
                    totalocc.push_back(occ1.at(i)); 
                }
            }

            // word1 and word2 or word3
            else if(terms[1]=="and" && terms[3]=="or")
            {
                set_intersection(occ1.begin(),occ1.end(),occ2.begin(),occ2.end(),back_inserter(terocc));
                for(i=0;i<terocc.size();i++)
                {
                    totalocc.push_back(terocc.at(i));    
                }
                for(i=0;i<occ3.size();i++)
                {
                    totalocc.push_back(occ3.at(i)); 
                }
            }

            // word1 or word2 or word3
            else if(terms[1]=="or" && terms[3]=="or")
            {
                for(i=0;i<occ1.size();i++)
                {
                    totalocc.push_back(occ1.at(i));    
                }
                for(i=0;i<occ2.size();i++)
                {
                    totalocc.push_back(occ2.at(i)); 
                }
                for(i=0;i<occ3.size();i++)
                {
                    totalocc.push_back(occ3.at(i)); 
                }
            }
            count.push_back(totalocc.size());

        }

        // Condition if a query is of length 6, i.e. it is the form
        // w1 and/or not w2 and/or w3  
        // 0    1    2   3    4  5
        //  --- OR of the form
        // w1 and/or w2 and/or not w3 
        // 0     1    2   3     4  5
        else if(querywords==6)
        {
            if(terms[2]=="not")
            {
                pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);

                for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
                {
                    // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                    for(i=0;i<occ1.size();i++)
                    {
                        if(occ1.at(i)==it1->second)
                        {
                            check1=1;
                        }
                    }
                    if(check1==0)
                    {
                        occ1.push_back(it1->second);
                    }
                    check1=0;
                }
                
                pair <MAPIterator, MAPIterator> result2 = dict.equal_range(terms[3]);

                for (MAPIterator it2 = result2.first; it2 != result2.second; it2++)
                {
                    // Repeating for second term
                    for(i=0;i<occ2.size();i++)
                    {
                        if(occ2.at(i)==it2->second)
                        {
                            check2=1;
                        }
                    }
                    if(check2==0)
                    {
                        occ2.push_back(it2->second);
                    }
                    check2=0;
                }

                pair <MAPIterator, MAPIterator> result3 = dict.equal_range(terms[5]);

                for (MAPIterator it3 = result3.first; it3 != result3.second; it3++)
                {
                    // Repeating for second term
                    for(i=0;i<occ3.size();i++)
                    {
                        if(occ3.at(i)==it3->second)
                        {
                            check3=1;
                        }
                    }
                    if(check3==0)
                    {
                        occ3.push_back(it3->second);
                    }
                    check3=0;
                }

                // with NOT in the middle
                // w1 and not w2 and w3
                if(terms[1]=="and" && terms[4]=="and")
                {
                    set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(terocc));
                    set_intersection(terocc.begin(),terocc.end(),occ1.begin(),occ1.end(),back_inserter(alphaocc));
                    set_intersection(alphaocc.begin(),alphaocc.end(),occ3.begin(),occ3.end(),back_inserter(totalocc));
                }
                // w1 or not w2 and w3
                else if(terms[1]=="or" && terms[4]=="and")
                {
                    set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(terocc));
                    set_intersection(terocc.begin(),terocc.end(),occ3.begin(),occ3.end(),back_inserter(alphaocc));
                    for(i=0;i<alphaocc.size();i++)
                    {
                        totalocc.push_back(alphaocc.at(i)); 
                    }
                    for(i=0;i<occ1.size();i++)
                    {
                        totalocc.push_back(occ1.at(i)); 
                    }
                }
                // w1 or not w2 or w3
                else if(terms[1]=="or" && terms[4]=="or")
                {
                    set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(terocc));
                    for(i=0;i<terocc.size();i++)
                    {
                        totalocc.push_back(terocc.at(i)); 
                    }
                    for(i=0;i<occ1.size();i++)
                    {
                        totalocc.push_back(occ1.at(i)); 
                    }
                    for(i=0;i<occ3.size();i++)
                    {
                        totalocc.push_back(occ3.at(i)); 
                    }
                }
                // w1 and not w2 or w3
                else if(terms[1]=="and" && terms[4]=="or")
                {
                    set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(terocc));
                    set_intersection(terocc.begin(),terocc.end(),occ1.begin(),occ1.end(),back_inserter(alphaocc));
                    for(i=0;i<alphaocc.size();i++)
                    {
                        totalocc.push_back(alphaocc.at(i)); 
                    }
                    for(i=0;i<occ3.size();i++)
                    {
                        totalocc.push_back(occ3.at(i)); 
                    }
                }
            }
            if(terms[4]=="not")
            {
                pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);

                for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
                {
                    // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                    for(i=0;i<occ1.size();i++)
                    {
                        if(occ1.at(i)==it1->second)
                        {
                            check1=1;
                        }
                    }
                    if(check1==0)
                    {
                        occ1.push_back(it1->second);
                    }
                    check1=0;
                }
                
                pair <MAPIterator, MAPIterator> result2 = dict.equal_range(terms[2]);

                for (MAPIterator it2 = result2.first; it2 != result2.second; it2++)
                {
                    // Repeating for second term
                    for(i=0;i<occ2.size();i++)
                    {
                        if(occ2.at(i)==it2->second)
                        {
                            check2=1;
                        }
                    }
                    if(check2==0)
                    {
                        occ2.push_back(it2->second);
                    }
                    check2=0;
                }

                pair <MAPIterator, MAPIterator> result3 = dict.equal_range(terms[5]);

                for (MAPIterator it3 = result3.first; it3 != result3.second; it3++)
                {
                    // Repeating for second term
                    for(i=0;i<occ3.size();i++)
                    {
                        if(occ3.at(i)==it3->second)
                        {
                            check3=1;
                        }
                    }
                    if(check3==0)
                    {
                        occ3.push_back(it3->second);
                    }
                    check3=0;
                }
                // with NOT at the end
                // w1 and w2 and not w3
                if(terms[1]=="and" && terms[3]=="and")
                {
                    set_difference(globalocc.begin(),globalocc.end(),occ3.begin(),occ3.end(),back_inserter(terocc));
                    set_intersection(terocc.begin(),terocc.end(),occ2.begin(),occ2.end(),back_inserter(alphaocc));
                    set_intersection(alphaocc.begin(),alphaocc.end(),occ1.begin(),occ1.end(),back_inserter(totalocc));
                }
                // w1 and w2 or not w3
                else if(terms[1]=="and" && terms[3]=="or")
                {
                    set_difference(globalocc.begin(),globalocc.end(),occ3.begin(),occ3.end(),back_inserter(terocc));
                    set_intersection(occ2.begin(),occ2.end(),occ1.begin(),occ1.end(),back_inserter(alphaocc));
                    for(i=0;i<alphaocc.size();i++)
                    {
                        totalocc.push_back(alphaocc.at(i)); 
                    }
                    for(i=0;i<terocc.size();i++)
                    {
                        totalocc.push_back(terocc.at(i)); 
                    }
                }
                // w1 or w2 and not w3
                else if(terms[1]=="or" && terms[3]=="and")
                {
                    set_difference(globalocc.begin(),globalocc.end(),occ3.begin(),occ3.end(),back_inserter(terocc));
                    set_intersection(occ2.begin(),occ2.end(),terocc.begin(),terocc.end(),back_inserter(alphaocc));
                    for(i=0;i<alphaocc.size();i++)
                    {
                        totalocc.push_back(alphaocc.at(i)); 
                    }
                    for(i=0;i<occ1.size();i++)
                    {
                        totalocc.push_back(occ1.at(i)); 
                    }
                }
                // w1 or w2 or not w3
                else if(terms[1]=="or" && terms[3]=="or")
                {
                    set_difference(globalocc.begin(),globalocc.end(),occ3.begin(),occ3.end(),back_inserter(terocc));
                    for(i=0;i<terocc.size();i++)
                    {
                        totalocc.push_back(terocc.at(i)); 
                    }
                    for(i=0;i<occ1.size();i++)
                    {
                        totalocc.push_back(occ1.at(i)); 
                    }
                    for(i=0;i<occ1.size();i++)
                    {
                        totalocc.push_back(occ2.at(i)); 
                    }
                }
            } 
            count.push_back(totalocc.size());
        }
        

        // Condition if a query is of length 6, i.e. it is the form
        // w1 and/or not w2 and/or not w3  
        // 0    1    2   3    4     5   6
        else if(querywords==7)
        {
            pair <MAPIterator, MAPIterator> result1 = dict.equal_range(terms[0]);

            for (MAPIterator it1 = result1.first; it1 != result1.second; it1++)
            {
                // Only storing document ID's that are unique in a vector to track all occurrences of term 1
                for(i=0;i<occ1.size();i++)
                {
                    if(occ1.at(i)==it1->second)
                    {
                        check1=1;
                    }
                }
                if(check1==0)
                {
                    occ1.push_back(it1->second);
                }
                check1=0;
            }
            
            pair <MAPIterator, MAPIterator> result2 = dict.equal_range(terms[3]);

            for (MAPIterator it2 = result2.first; it2 != result2.second; it2++)
            {
                // Repeating for second term
                for(i=0;i<occ2.size();i++)
                {
                    if(occ2.at(i)==it2->second)
                    {
                        check2=1;
                    }
                }
                if(check2==0)
                {
                    occ2.push_back(it2->second);
                }
                check2=0;
            }

            pair <MAPIterator, MAPIterator> result3 = dict.equal_range(terms[6]);

            for (MAPIterator it3 = result3.first; it3 != result3.second; it3++)
            {
                // Repeating for second term
                for(i=0;i<occ3.size();i++)
                {
                    if(occ3.at(i)==it3->second)
                    {
                        check3=1;
                    }
                }
                if(check3==0)
                {
                    occ3.push_back(it3->second);
                }
                check3=0;
            }
            // 0   1   2   3  4   5  6
            // w1 and not w2 and not w3
            if(terms[1]=="and" && terms[4]=="and")
            {
                set_difference(globalocc.begin(),globalocc.end(),occ3.begin(),occ3.end(),back_inserter(alphaocc));
                set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(betaocc));

                set_intersection(betaocc.begin(),betaocc.end(),alphaocc.begin(),alphaocc.end(),back_inserter(terocc));
                set_intersection(terocc.begin(),terocc.end(),occ1.begin(),occ1.end(),back_inserter(totalocc));
            }
            // 0   1   2   3  4   5  6
            // w1 and not w2 or not w3
            else if(terms[1]=="and" && terms[4]=="or")
            {
                set_difference(globalocc.begin(),globalocc.end(),occ3.begin(),occ3.end(),back_inserter(alphaocc));
                set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(betaocc));

                set_intersection(betaocc.begin(),betaocc.end(),occ1.begin(),occ1.end(),back_inserter(terocc));
                for(i=0;i<alphaocc.size();i++)
                {
                    totalocc.push_back(alphaocc.at(i)); 
                }
                for(i=0;i<terocc.size();i++)
                {
                    totalocc.push_back(terocc.at(i)); 
                }
            }
            // 0   1   2   3  4   5  6
            // w1 or not w2 and not w3
            else if(terms[1]=="or" && terms[4]=="and")
            {
                set_difference(globalocc.begin(),globalocc.end(),occ3.begin(),occ3.end(),back_inserter(alphaocc));
                set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(betaocc));

                set_intersection(alphaocc.begin(),alphaocc.end(),betaocc.begin(),betaocc.end(),back_inserter(terocc));
                for(i=0;i<occ1.size();i++)
                {
                    totalocc.push_back(occ1.at(i)); 
                }
                for(i=0;i<terocc.size();i++)
                {
                    totalocc.push_back(terocc.at(i)); 
                }
            }
            // 0   1   2   3  4   5  6
            // w1 or not w2 or not w3
            else if(terms[1]=="or" && terms[4]=="or")
            {
                set_difference(globalocc.begin(),globalocc.end(),occ3.begin(),occ3.end(),back_inserter(alphaocc));
                set_difference(globalocc.begin(),globalocc.end(),occ2.begin(),occ2.end(),back_inserter(betaocc));

                for(i=0;i<alphaocc.size();i++)
                {
                    totalocc.push_back(alphaocc.at(i)); 
                }
                for(i=0;i<betaocc.size();i++)
                {
                    totalocc.push_back(betaocc.at(i)); 
                }
                for(i=0;i<occ1.size();i++)
                {
                    totalocc.push_back(occ1.at(i)); 
                }

            }
            count.push_back(totalocc.size());
        }

        // Sorting the vector 

        sort(totalocc.begin(),totalocc.end());

        // Erasing all duplicate values (if there are any, one last time as a final check) using the STL unique function
        totalocc.erase(unique(totalocc.begin(),totalocc.end()),totalocc.end());

        // Clearing count and storing the new size after erasing dupes and sorting
        count.clear();
        count.push_back(totalocc.size());

        // Accumulating the values inside the count vector
        int freq=0;
        freq=accumulate(count.begin(),count.end(),0);
        
        if(freq!=0)
        {
            cout<<"The query \""<<query<<"\" has occurred in the following documents: "<<endl;

            // Displaying results from the vector totalocc
            for(i=0;i<totalocc.size();i++)
            {
                if(i==totalocc.size()-1)
                {
                    cout<<totalocc.at(i);
                }
                else
                {
                    cout<<totalocc.at(i)<<", ";
                }
            }
        }
        
        // If the vector count is empty, no results were stored, therefore no occurrences exist of the query.
        else
        {
            cout<<"The query \""<<query<<"\" has occured 0 times";
        }
        count.clear();
        cout<<endl;
        cout<<endl;
    }
}