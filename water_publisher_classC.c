
/* water_publisher.c

A publication of data of type WaterDispenser

This file is derived from code automatically generated by the rtiddsgen 
command:

rtiddsgen -language C -example <arch> water.idl

Example publication of type WaterDispenser automatically generated by 
'rtiddsgen'. To test them follow these steps:

(1) Compile this file and the example subscription.

(2) Start the subscription on the same domain used for RTI Data Distribution
Service with the command
objs/<arch>/water_subscriber <domain_id> <sample_count>

(3) Start the publication on the same domain used for RTI Data Distribution
Service with the command
objs/<arch>/water_publisher <domain_id> <sample_count>

(4) [Optional] Specify the list of discovery initial peers and 
multicast receive addresses via an environment variable or a file 
(in the current working directory) called NDDS_DISCOVERY_PEERS. 

You can run any number of publishers and subscribers programs, and can 
add and remove them dynamically from the domain.

Example:

To run the example application on domain <domain_id>:

On Unix: 

objs/<arch>/water_publisher <domain_id> 
objs/<arch>/water_subscriber <domain_id> 

On Windows:

objs\<arch>\water_publisher <domain_id>  
objs\<arch>\water_subscriber <domain_id>   
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ndds/ndds_c.h"
#include "water_seldom.h"
#include "water_seldomSupport.h"

/* Delete all entities */
static int publisher_shutdown(
    DDS_DomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = DDS_DomainParticipant_delete_contained_entities(participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDS_DomainParticipantFactory_delete_participant(
            DDS_TheParticipantFactory, participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Data Distribution Service provides finalize_instance() method on
    domain participant factory for people who want to release memory used
    by the participant factory. Uncomment the following block of code for
    clean destruction of the singleton. */
    /*
    retcode = DDS_DomainParticipantFactory_finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "finalize_instance error %d\n", retcode);
        status = -1;
    }
    */

    return status;
}

int publisher_main(int domainId, int sample_count)
{
    DDS_DomainParticipant *participant = NULL;
    DDS_Publisher *publisher = NULL;
    DDS_Topic *topic = NULL;
    DDS_DataWriter *writer = NULL;
    WaterDispenserDataWriter *WaterDispenser_writer = NULL;
    //WaterDispenserDataWriter *WaterDispenser_writer1 = NULL;
    WaterDispenser *instance = NULL;
    WaterDispenser *instance1 = NULL;
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    DDS_InstanceHandle_t instance_handle1 = DDS_HANDLE_NIL;
    const char *type_name = NULL;
    int count = 0;  
    //struct DDS_Duration_t poll_period = {1,0};
    struct DDS_Duration_t send_period = {1,0};
    
    /* To customize participant QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    participant = DDS_DomainParticipantFactory_create_participant_with_profile(
        DDS_TheParticipantFactory, domainId, "TCP_Library", "TCP_Publisher",
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        fprintf(stderr, "create_participant error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize publisher QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    publisher = DDS_DomainParticipant_create_publisher(
        participant, &DDS_PUBLISHER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        fprintf(stderr, "create_publisher error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* Register type before creating topic */
    type_name = WaterDispenserTypeSupport_get_type_name();
    retcode = WaterDispenserTypeSupport_register_type(
        participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "register_type error %d\n", retcode);
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize topic QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    topic = DDS_DomainParticipant_create_topic(
        participant, "Example WaterDispenser",
        type_name, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        fprintf(stderr, "create_topic error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize data writer QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    writer = DDS_Publisher_create_datawriter_with_profile(
        publisher, topic,
        "TCP_Library", "TCP_ProfileBase", NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
        fprintf(stderr, "create_datawriter error\n");
        publisher_shutdown(participant);
        return -1;
    }
    WaterDispenser_writer = WaterDispenserDataWriter_narrow(writer);
    if (WaterDispenser_writer == NULL) {
        fprintf(stderr, "DataWriter narrow error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* Create data sample for writing */
    instance1 = WaterDispenserTypeSupport_create_data_ex(DDS_BOOLEAN_TRUE);
    if (instance1 == NULL) {
        fprintf(stderr, "WaterDispenserTypeSupport_create_data1 error\n");
        publisher_shutdown(participant);
        return -1;
    }
    instance = WaterDispenserTypeSupport_create_data_ex(DDS_BOOLEAN_TRUE);
    if (instance == NULL) {
        fprintf(stderr, "WaterDispenserTypeSupport_create_data error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* For a data type that has a key, if the same instance is going to be
    written multiple times, initialize the key here
    and register the keyed instance prior to writing */
    
    instance_handle = WaterDispenserDataWriter_register_instance(
        WaterDispenser_writer, instance);
    // instance_handle1 = WaterDispenserDataWriter_register_instance(
    //     WaterDispenser_writer1, instance1);
    /* Main loop */
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {
        NDDS_Utility_sleep(&send_period);
        printf("Writing WaterDispenser, count %d\n", count);
        
        /* Modify the data to be written here */
        //for read data
        typedef struct{
            char* s;
            struct node* next;
        }node;
        /* Modify the data to be written here */
        ////
        
        FILE *file;
        char buffer[BUFSIZ];
        int n = 0;
        int c;
        const char * const delim = ",";
        //printf("ok");
 
        file = fopen ("/home/T20501/DDS/dispenser_pi/DDS_Data.csv", "r");
        if (!file) {
                perror("error to open file");
                return EXIT_FAILURE;
        }
        //do
        //{
            
            char substrarray[300];
            const size_t line_size = 300;
            char* line =(char*) malloc(line_size);
            char *token;
            node* lines[1024];
            int count=0;
            while (fgets(line, line_size, file) != NULL)  {
                //printf(line);
            
               
            //for (int i=0; i<2 ;i++)
            //{
                char* temp=(char*) malloc(line_size);
                //c=fgets(line, line_size, file);
                strcpy(temp,line);
                
                ///
                //printf("%s\n",temp);
                token=strtok(temp, delim);
                node* nd=(node*)malloc(sizeof(node));
                nd->next=NULL;  
                lines[count]=nd;
                if(token)
                {
                    nd->s=token;
                }
                //printf("%s",token);
                do
                {
                    
                    //nd.s=(char*) malloc(line_size);
                    token=strtok(NULL, delim);
                    if(token)
                    {
                        nd->next=(node*)malloc(sizeof(node));
                        nd=nd->next;
                        nd->s=token;
                        nd->next=NULL;
                    }
                    //printf("%s",token);
                }while(token);

                count++;
               
            }
            /*node* l=lines[1];      
            while(l!=NULL)
            {
                printf("%s ",l->s);
                l=l->next;
            }*/
            int j=1;
            node* l;
            
            
            struct DDS_Time_t current;
            DDS_DomainParticipant_get_current_time(participant, &current); 
            long long tolong=current.sec;
                         

            instance-> SendTime=tolong*1000+current.nanosec/1000000;     

                l=lines[0];
                l=l->next;
                instance-> UploadTime=l->s;
               
                l=lines[5];
                l=l->next;
                instance-> ColdTemp_Insulation=atol(l->s);

                l=lines[7];
                l=l->next;
                instance-> Usage_MT=atol(l->s);

                l=lines[9];
                l=l->next;
                instance-> ErrorCode=atol(l->s);

                l=lines[10];
                l=l->next;
                instance-> HotTemp_Insulation=atol(l->s);
                //int HotTemp_Insulation=instance-> HotTemp_Insulation;
                
                l=lines[11];
                l=l->next;
                instance-> Device_ID=l->s;

                l=lines[13];
                l=l->next;
                instance-> SavingPower=atol(l->s);

                l=lines[14];
                l=l->next;
                instance-> TimeStamp=atoll(l->s);
                
                l=lines[21];
                l=l->next;
                instance-> Sterilizing=atol(l->s);

                l=lines[23];
                l=l->next;
                instance-> Mac_Address=l->s;

                // l=lines[1];
                // l=l->next;
                // printf("%s",l->s);
            /*printf("%s ",l->s);
            l=l->next;
            printf("%s ",l->s);
            l=l->next;*/
            //if(l==NULL) printf("NO");
            //printf("%s", lines[2][0]?:'-');

        fclose (file);
        ////
        
       typedef struct{
                    char* ss;
                    struct node1* next1;
        }node1;       
        FILE *file1;
        char buffer1[BUFSIZ];
        int n1 = 0;
        int c1;
        //const char * const delim = ",";
        // FILE *file0 = fopen
        // if (file0 == NULL) return -1; 
        //     //while (count-- > 0) { 
        //         // you might want to check for out-of-disk-space here, too 
        // fprintf(file0, "%s,%d\n","HotTemp_Insulation",instance-> HotTemp_Insulation); 
        // fprintf(file0, "%s,%d\n","ColdTemp_Insulation",instance-> ColdTemp_Insulation); 
                
        //     //} 
        // fclose(file0);
        //char file_name2[] = ;
                    
        file1 = fopen("DDS_Data_old.csv", "r");
        if (!file1) {
                        perror("error to open file");
                        return EXIT_FAILURE;
                }
        //do
        //{
        
        char substrarray1[300];
        const size_t line_size1 = 300;
        char* line1 =(char*) malloc(line_size1);
        char *token1;
        node1* lines1[1024];
        int count1=0;
        while (fgets(line1, line_size1, file1) != NULL)  {
                //printf(line);
            //printf(line1);   
               
            //for (int i=0; i<2 ;i++)
            //{
                char* temp1=(char*) malloc(line_size1);
                //c=fgets(line, line_size, file);
                strcpy(temp1,line1);
                
                ///
                //printf("%s\n",temp);
                token1=strtok(temp1, delim);
                node1* nd1=(node1*)malloc(sizeof(node1));
                nd1->next1=NULL;    
                lines1[count1]=nd1;
                if(token1)
                {
                    nd1->ss=token1;
                }
                //printf("%s",token);
                do
                {
                    
                    token1=strtok(NULL, delim);
                    if(token1)
                    {
                        nd1->next1=(node1*)malloc(sizeof(node1));
                        nd1=nd1->next1;
                        nd1->ss=token1;
                        nd1->next1=NULL;
                    }
                }while(token1);

                count1++;
               
            }
            int jj=1;
            node1* ll;
            ll=lines1[0];
            ll=ll->next1;
            instance1-> HotTemp_Insulation=atol(ll->ss);

            ll=lines1[1];
            ll=ll->next1;
            instance1-> ColdTemp_Insulation=atol(ll->ss);

            ll=lines1[2];
            ll=ll->next1;
            instance1-> Usage_MT=atol(ll->ss);

            ll=lines1[3];
            ll=ll->next1;
            instance1-> ErrorCode=atol(ll->ss);

            ll=lines1[4];
            ll=ll->next1;
            instance1-> SavingPower=atol(ll->ss);

            ll=lines1[5];
            ll=ll->next1;
            instance1-> Sterilizing=atol(ll->ss);


            // int ColdTemp_Insulation=instance1-> ColdTemp_Insulation;
            // int HotTemp_Insulationcompare=instance1-> HotTemp_Insulation;
            //printf("%s,%d\n","HotTemp_Insulation1",HotTemp_Insulationcompare);  
        fclose (file1);
            
        if ((instance-> Sterilizing != instance1-> Sterilizing)||(instance-> SavingPower != instance1-> SavingPower)||
        (instance-> ErrorCode != instance1-> ErrorCode)||(instance-> Usage_MT != instance1-> Usage_MT)||
        (instance-> HotTemp_Insulation != instance1-> HotTemp_Insulation)||(instance-> ColdTemp_Insulation != instance1-> ColdTemp_Insulation))
        {
        /* Write data */
            
            // retcode = WaterDispenserDataWriter_write(
            //     WaterDispenser_writer1, instance1, &instance_handle1);
            // if (retcode != DDS_RETCODE_OK) {
            //     fprintf(stderr, "write error %d\n", retcode);
            // }
            retcode = WaterDispenserDataWriter_write(
                WaterDispenser_writer, instance, &instance_handle);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "write error %d\n", retcode);
            }

            //instance1-> HotTemp_Insulation=atol(l->s);
            
            FILE *file2 = fopen("DDS_Data_old.csv", "w"); 
            if (file2 == NULL) return -1; 
            //while (count-- > 0) { 
                // you might want to check for out-of-disk-space here, too 
            fprintf(file2, "%s,%d\n","HotTemp_Insulation",instance-> HotTemp_Insulation); 
            fprintf(file2, "%s,%d\n","ColdTemp_Insulation",instance-> ColdTemp_Insulation); 
            fprintf(file2, "%s,%d\n","Usage_MT",instance-> Usage_MT); 
            fprintf(file2, "%s,%d\n","ErrorCode",instance-> ErrorCode); 
            fprintf(file2, "%s,%d\n","SavingPower",instance-> SavingPower); 
            fprintf(file2, "%s,%d\n","Sterilizing",instance-> Sterilizing);     
            //} 
            fclose(file2);
          
        }
        //printf("data1 %d,data2 %d\n",instance-> HotTemp_Insulation ,instance1-> HotTemp_Insulation);
        //NDDS_Utility_sleep(&send_period); //should disable send period?  
        
    }

    /*
    retcode = WaterDispenserDataWriter_unregister_instance(
        WaterDispenser_writer, instance, &instance_handle);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "unregister instance error %d\n", retcode);
    }
    */
   retcode = WaterDispenserTypeSupport_delete_data_ex(instance1, DDS_BOOLEAN_TRUE);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "WaterDispenserTypeSupport_delete_data error %d\n", retcode);
            }
    /* Delete data sample */
    retcode = WaterDispenserTypeSupport_delete_data_ex(instance, DDS_BOOLEAN_TRUE);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "WaterDispenserTypeSupport_delete_data error %d\n", retcode);
    }
    
    /* Cleanup and delete delete all entities */         
    return publisher_shutdown(participant);
}

int main(int argc, char *argv[])
{
    int domain_id = 0;
    int sample_count = 0; /* infinite loop */
/*
    if (argc >= 2) {
        domain_id = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }
*/
    /* Uncomment this to turn on additional logging
    NDDS_Config_Logger_set_verbosity_by_category(
        NDDS_Config_Logger_get_instance(),
        NDDS_CONFIG_LOG_CATEGORY_API, 
        NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */

    return publisher_main(domain_id, sample_count);
}
