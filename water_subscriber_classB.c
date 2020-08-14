#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ndds/ndds_c.h"
#include "water_regular.h"
#include "water_regularSupport.h"
#include <bson.h>
#include <bcon.h>
#include <mongoc.h>
#include <time.h>

char *UploadTime;
char *Device_ID;
long long TimeStamp;
long long SendTime;
char *Mac_Address;

long Hot_Valve;
long Warm_Valve;
long Cold_Valve;

long Heating;
long Cooling;
long Refilling;

long Usage_L;
 
/** Delete all entities */
static int subscriber_shutdown(
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

    /** RTI Data Distribution Service provides the finalize_instance() method on
    domain participant factory for users who want to release memory used
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

static int subscriber_main(int domainId, int sample_count)
{
    DDS_DomainParticipant *participant = NULL;
    DDS_Subscriber *subscriber = NULL;
    DDS_Topic *topic = NULL;
    DDS_DataReader *reader = NULL;
    DDS_ReturnCode_t retcode;
    const char *type_name = NULL;
    int count = 0;
    DDS_StatusCondition *status_condition;
    DDS_WaitSet *waitset=NULL;
    WaterDispenserDataReader *WaterDispenserReader=NULL; 
    struct DDS_Duration_t timeout = {1,500000000};
  
    /** Function to create domain participant based on 
    a Qos Profiles set on USER_QOS_PROFILES.xml
     @param self          <<in>> Cannot be NULL.
     @param domainId      <<in>> ID of the domain that the application intends to join. [range] [>=0], and does not violate guidelines stated in DDS_RtpsWellKnownPorts_t.
     @param library_name  <<in>> Library name containing the XML QoS profile. If library_name is null RTI Connext will use the default library (see DDS_DomainParticipantFactory_set_default_library).
     @param profile_name  <<in>> XML QoS Profile name. If profile_name is null RTI Connext will use the default profile (see DDS_DomainParticipantFactory_set_default_profile).
     @param listener      <<in>> the DomainParticipant's listener.
     @param mask          <<in>>. Changes of communication status to be invoked on the listener. See DDS_StatusMask.
     @return domainParticipant or NULL on failure
     @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.2.3/doc/api/connext_dds/api_c/group__DDSDomainParticipantFactoryModule.html#gaccf76843ecbd588aea706cdf6aa6aa83">More Info</a>
    */
   
    participant = DDS_DomainParticipantFactory_create_participant_with_profile(
        DDS_TheParticipantFactory, domainId, "TCP_Library", "TCP_Subscriber",
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        fprintf(stderr, "create_participant error\n");
        subscriber_shutdown(participant);
        return -1;
    }
    
     /** Function to create domain participant based on a Qos Profiles set on USER_QOS_PROFILES.xml
     @param self     <<in>> Cannot be NULL.
     @param qos      <<in>> QoS to be used for creating the new DDS_Publisher. The special value DDS_PUBLISHER_QOS_DEFAULT can be used to indicate that the DDS_Publisher should be created with the default DDS_PublisherQos set in the DDS_DomainParticipant. Cannot be NULL.
     @param listener <<in>>. Listener to be attached to the newly created DDS_Publisher.
     @param mask     <<in>>. Changes of communication status to be invoked on the listener. See DDS_StatusMask.
     @return newly created publisher object or NULL on failure.
     @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.2.3/doc/api/connext_dds/api_c/group__DDSDomainParticipantModule.html#ga59c1e6c22bb71b5b7d294ca1b669ce63">More Info</a>
    */
     subscriber = DDS_DomainParticipant_create_subscriber(
        participant, &DDS_SUBSCRIBER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        fprintf(stderr, "create_subscriber error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /** Function to get Type Name (idl struct name) */
    type_name = WaterDispenserTypeSupport_get_type_name();//WaterDispenserTypeSupport->waitset_statuscondTypeSupport
    
     /** Function to Allows an application to communicate to RTI Connext the existence of the char* data type.
    @param participant <<in>> the DDSDomainParticipant to unregister the data type char* from. Cannot be NULL.
    @param type_name   <<in>> the type name under with the data type char* is registered with the participant. The name should match a name that has been previously used to register a type with the participant. Cannot be NULL.
    @return One of the Standard Return Codes, DDS_RETCODE_BAD_PARAMETER or DDS_RETCODE_ERROR
    @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_cpp/classDDSStringTypeSupport.html#">More Info</a>
    */
   
    retcode = WaterDispenserTypeSupport_register_type(participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "register_type error %d\n", retcode);
        subscriber_shutdown(participant);
        return -1;
    }

     /** Function to Create Topic with the desired Qos Policies
      @param self  <<in>> Cannot be NULL.
      @param topic_name  <<in>> Name for the new topic, must not exceed 255 characters. Cannot be NULL.
      @param type_name   <<in>> The type to which the new DDS_Topic will be bound. Cannot be NULL.
      @param qos   <<in>> QoS to be used for creating the new DDS_Topic. The special value DDS_TOPIC_QOS_DEFAULT can be used to indicate that the DDS_Topic should be created with the default DDS_TopicQos set in the DDS_DomainParticipant. Cannot be NULL.
      @param listener  <<in>>. Listener to be attached to the newly created DDS_Topic.
      @param mask  <<in>>. Changes of communication status to be invoked on the listener.
      @return newly created topic, or NULL on failure
      @brief <a href="http://community.rti.com/rti-doc/45e/ndds.4.5e/doc/html/api_c/group__DDSDomainParticipantModule.html#g1b785962ec5a94098875b2268d1655c3">More Info</a>
    */
    topic = DDS_DomainParticipant_create_topic(
        participant, "Example WaterDispenser",
        type_name, &DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        fprintf(stderr, "create_topic error\n");
        subscriber_shutdown(participant);
        return -1;
    }


     /** Function to Creates a DDS_DataReader that will be attached and belong to the implicit DDS_Subscriber
     @param self      <<in>> Cannot be NULL.
     @param topic     <<in>> The DDS_TopicDescription that the DDS_DataReader will be associated with. Cannot be NULL.
     @param qos       <<in>> The qos of the DDS_DataReader. The special value DDS_DATAREADER_QOS_DEFAULT can be used to indicate that the DDS_DataReader should be created with the default DDS_DataReaderQos set in the implicit DDS_Subscriber. If DDS_TopicDescription is of type DDS_Topic or DDS_ContentFilteredTopic, the special value DDS_DATAREADER_QOS_USE_TOPIC_QOS can be used to indicate that the DDS_DataReader should be created with the combination of the default DDS_DataReaderQos set on the implicit DDS_Subscriber and the DDS_TopicQos (in the case of a DDS_ContentFilteredTopic, the DDS_TopicQos of the related DDS_Topic). if DDS_DATAREADER_QOS_USE_TOPIC_QOS is used, topic cannot be a DDS_MultiTopic. Cannot be NULL.
     @param listener  <<in>> The listener of the DDS_DataReader.
     @param mask      <<in>>. Changes of communication status to be invoked on the listener.
     @return A DDS_DataReader of a derived class specific to the data-type associated with the DDS_Topic or NULL if an error occurred.
     @brief <a href="http://community.rti.com/rti-doc/45e/ndds.4.5e/doc/html/api_c/group__DDSDomainParticipantModule.html#g33e5507940fbe7fe83a9e224008cca58">More Info</a>
    */

    reader = DDS_Subscriber_create_datareader_with_profile(
        subscriber, DDS_Topic_as_topicdescription(topic),
        "TCP_Library", "TCP_Subscriber", NULL/*&reader_listener*/, DDS_STATUS_MASK_ALL);
    if (reader == NULL) {
        fprintf(stderr, "create_datareader error\n");
        subscriber_shutdown(participant);
        return -1;
    
    }


    /** Function to Allows access to the DDSStatusCondition associated with the DDSEntity.
     @return the status condition associated with this entity.
     @brief <a href="http://community.rti.com/rti-doc/510/ndds/doc/html/api_cpp/classDDSEntity.html#">More Info</a>
    */

    status_condition= DDS_Entity_get_statuscondition((DDS_Entity *) reader);
    if(status_condition==NULL){
        fprintf(stderr, "get_statuscondition error\n");
        subscriber_shutdown(participant);
        return -1;

    }

    /** Since a single status condition can match many statuses,
     enable only those we're interested in.*/

    /** This Function defines the list of communication statuses that determine the trigger_value of the DDS_StatusCondition.
     @param self  <<in>> Cannot be NULL.
     @param mask  <<in>> the list of enables statuses (see Status Kinds)
     @return One of the Standard Return Codes
     @brief <a href="https://community.rti.com/rti-doc/510/ndds.5.1.0/doc/html/api_c/group__DDSConditionsModule.html#">More Info</a>
    */
    retcode = DDS_StatusCondition_set_enabled_statuses(
            status_condition,
            DDS_DATA_AVAILABLE_STATUS |
            DDS_LIVELINESS_CHANGED_STATUS |
            DDS_REQUESTED_DEADLINE_MISSED_STATUS |
            DDS_SAMPLE_LOST_STATUS |
            DDS_SAMPLE_REJECTED_STATUS|
            DDS_SUBSCRIPTION_MATCHED_STATUS
            );
    if (retcode != DDS_RETCODE_OK) {
        printf("set_enabled_statuses error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /** This Function Construct a new DDS_WaitSet.
     @return A new DDS_WaitSet or NULL if one could not be allocated.
     @brief <a href="https://community.rti.com/rti-doc/500/ndds.5.0.0/doc/html/api_c/group__DDSConditionsModule.html#">More Info</a>
    */    
    waitset = DDS_WaitSet_new();
    if (waitset == NULL) {
        printf("create waitset error\n");
        subscriber_shutdown(participant);
        return -1;
    }

    /** This Function Attaches a DDS_Condition to the DDS_WaitSet.
     @param self <<in>> Cannot be NULL.
     @param cond  <<in>> Contition to be attached.
     @return One of the Standard Return Codes, or DDS_RETCODE_OUT_OF_RESOURCES.
     @brief <a href="https://community.rti.com/rti-doc/500/ndds.5.0.0/doc/html/api_c/group__DDSConditionsModule.html#">More Info</a>
    */
    retcode = DDS_WaitSet_attach_condition(
            waitset,
            (DDS_Condition *) status_condition);
    if (retcode != DDS_RETCODE_OK) {
        printf("attach_condition error\n");
        subscriber_shutdown(participant);
        return -1;
    }

     /** This Function Access a collection of data samples from the DDS_DataReader.
     @param self            <<in>> Cannot be NULL.
     @param received_data   <<inout>> User data type-specific FooSeq object where the received data samples will be returned. Must be a valid non-NULL FooSeq. The function will fail with DDS_RETCODE_BAD_PARAMETER if it is NULL.
     @param info_seq        <<inout>> A DDS_SampleInfoSeq object where the received sample info will be returned. Must be a valid non-NULL DDS_SampleInfoSeq. The function will fail with DDS_RETCODE_BAD_PARAMETER if it is NULL.
     @param max_samples     <<in>> The maximum number of samples to be returned. If the special value DDS_LENGTH_UNLIMITED is provided, as many samples will be returned as are available, up to the limits described in the documentation for FooDataReader_take().
     @param sample_states   <<in>> Data samples matching one of these sample_states are returned. See the valid values for this parameter here: DDS_SampleStateKind.
     @param view_states     <<in>> Data samples matching one of these view_state are returned. See the valid values for this parameter here: DDS_ViewStateKind.
     @param instance_states <<in>> Data samples matching ones of these instance_state are returned. See the valid values for this parameter here: DDS_InstanceStateKind.
     @return One of the Standard Return Codes, DDS_RETCODE_PRECONDITION_NOT_MET, DDS_RETCODE_NO_DATA or DDS_RETCODE_NOT_ENABLED.
     @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.2.3/doc/api/connext_dds/api_c/group__DDSReaderModule.html#">More Info</a>
    */
    WaterDispenserReader = WaterDispenserDataReader_narrow(reader);//waitset_statuscondDataReader_narrow(reader);
    if (WaterDispenserReader == NULL) {
        printf("DataReader narrow error\n");
        return -1;
    }
    ///db start
    mongoc_client_t      *client;
    mongoc_database_t    *database;
    mongoc_collection_t  *collection;
    bson_t              *command,
                            reply,
                            *insert;
    bson_error_t          error;
    char                *str;
    bool                  retval;
    const char* mongo_address = getenv("MONGO_ADDRESS");

  
    /** Function to create new mongoDB Client Session
    */
    client = mongoc_client_new (mongo_address);///创建连接对象
    database = mongoc_client_get_database (client, "water");///获取数据库
    collection = mongoc_client_get_collection (client, "water", "dispenser_data_always");///获取指定数据库和集合
    command = BCON_NEW ("ping", BCON_INT32 (1));
    retval = mongoc_client_command_simple (client, "admin", command, NULL, &reply, &error);///执行命令
    if (!retval) {
        fprintf (stderr, "%s\n", error.message);
        return EXIT_FAILURE;
    }
    str = bson_as_json (&reply, NULL);
    printf ("%s\n", str);
    
    
    /** Main loop */
    ///
    bson_t *water_doc;

    struct WaterDispenserSeq data_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq info_seq = DDS_SEQUENCE_INITIALIZER;
    struct DDS_ConditionSeq active_conditions = DDS_SEQUENCE_INITIALIZER;
    int i, j; 
    
    ///
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {
            struct DDS_Time_t current;
                    char *str_replace(char *orig, char *rep, char *with) {
                    char *result; /// the return string
                    char *ins;    /// the next insert point
                    char *tmp;    /// varies
                    int len_rep;  /// length of rep (the string to remove)
                    int len_with; /// length of with (the string to replace rep with)
                    int len_front; /// distance between rep and end of last rep
                    int count;    /// number of replacements

                    /// sanity checks and initialization
                    if (!orig || !rep)
                        return NULL;
                    len_rep = strlen(rep);
                    if (len_rep == 0)
                        return NULL; /// empty rep causes infinite loop during count
                    if (!with)
                        with = "";
                    len_with = strlen(with);

                    /// count the number of replacements needed
                    ins = orig;
                    for (count = 0; tmp = strstr(ins, rep); ++count) {
                        ins = tmp + len_rep;
                    }

                    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

                    if (!result)
                        return NULL;

                    /// first time through the loop, all the variable are set correctly
                    /// from here on,
                    ///    tmp points to the end of the result string
                    ///    ins points to the next occurrence of rep in orig
                    ///    orig points to the remainder of orig after "end of rep"
                    while (count--) {
                        ins = strstr(orig, rep);
                        len_front = ins - orig;
                        tmp = strncpy(tmp, orig, len_front) + len_front;
                        tmp = strcpy(tmp, with) + len_with;
                        orig += len_front + len_rep; /// move to next "end of rep"
                    }
                    strcpy(tmp, orig);
                    return result;
                }
               
            retcode = DDS_WaitSet_wait(waitset, &active_conditions, &timeout);
            if (retcode == DDS_RETCODE_TIMEOUT) {
                printf("wait timed out\n");
                count += 2;
                continue;
            } else if (retcode != DDS_RETCODE_OK) {
                printf("wait returned error: %d\n", retcode);
                break;
            }
            printf("got %d active conditions\n",
               DDS_ConditionSeq_get_length(&active_conditions));

             for (i = 0; i < DDS_ConditionSeq_get_length(&active_conditions); ++i) {
                /** Compare with Status Conditions */
                if (DDS_ConditionSeq_get(&active_conditions, i) == (DDS_Condition *) status_condition) {
                    
                    DDS_StatusMask triggeredmask;
                  
                    /** This Function Retrieves the list of communication statuses in the DDS_Entity that are triggered.
                     @param self              <<in>> Cannot be NULL.
                     @return list of communication statuses in the DDS_Entity that are triggered.
                     @brief <a href="https://community.rti.com/rti-doc/500/ndds.5.0.0/doc/html/api_c/group__DDSEntityModule.html#">More Info</a>
                    */
                    triggeredmask = DDS_Entity_get_status_changes(
                        (DDS_Entity *) WaterDispenserReader);

             
                     /** Subscription matched */
                    if (triggeredmask  & DDS_DATA_AVAILABLE_STATUS ) {
                        /** Current conditions match our conditions to read data, so
                        * we can read data just like we would do in any other
                        * example.
                        */
                        struct WaterDispenserSeq data_seq = DDS_SEQUENCE_INITIALIZER;
                        struct DDS_SampleInfoSeq info_seq = DDS_SEQUENCE_INITIALIZER;

                        /** Access data using read(), take(), etc.  If you fail to do
                        * this the condition will remain true, and the WaitSet will
                        * wake up immediately - causing high CPU usage when it does
                        * not sleep in the loop */
                        retcode = DDS_RETCODE_OK;
                      while (retcode != DDS_RETCODE_NO_DATA) {
                            /** This Function Retrieves the list of communication statuses in the DDS_Entity that are triggered.
                             @param received_data <<inout>> User data type-specific FooSeq object where the received data samples will be returned. Must be a valid non-NULL FooSeq. The function will fail with DDS_RETCODE_BAD_PARAMETER if it is NULL.
                             @param self  <<in>> Cannot be NULL.
                             @param info_seq  <<inout>> A DDS_SampleInfoSeq object where the received sample info will be returned. Must be a valid non-NULL DDS_SampleInfoSeq. The function will fail with DDS_RETCODE_BAD_PARAMETER if it is NULL.
                             @param max_samples <<in>> The maximum number of samples to be returned. If the special value DDS_LENGTH_UNLIMITED is provided, as many samples will be returned as are available, up to the limits described above.
                             @param sample_states <<in>> Data samples matching one of these sample_states are returned.
                             @param view_states <<in>> Data samples matching one of these view_state are returned.
                             @param instance_states <<in>> Data samples matching one of these instance_state are returned.
                             @return list of communication statuses in the DDS_Entity that are triggered.
                             @brief <a href="https://community.rti.com/rti-doc/500/ndds.5.0.0/doc/html/api_c/group__DDSEntityModule.html#">More Info</a>
                            */
                            retcode = WaterDispenserDataReader_take(
                                    WaterDispenserReader,
                                    &data_seq,
                                    &info_seq,
                                    DDS_LENGTH_UNLIMITED,
                                    DDS_ANY_SAMPLE_STATE,
                                    DDS_ANY_VIEW_STATE,
                                    DDS_ANY_INSTANCE_STATE);
                            
                             /** ( WaterDispenserSeq_get_length )This Function Get the logical length of this sequence.
                             @param self <<in>> Cannot be NULL.
                             @return the length of the sequence
                             @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.3.0/doc/api/connext_dds/api_c/group__DDSSequenceModule.html#">More Info</a>
                            */
                            for (j = 0;
                                j < WaterDispenserSeq_get_length(&data_seq);
                                ++j) {
                                if (!DDS_SampleInfoSeq_get_reference(&info_seq, j)
                                            ->valid_data) {
                                    printf("   Got metadata\n");
                                    continue;
                                }
                                /** ( WaterDispenserSeq_get_reference )This Function Get the pointer to the i-th element of this sequence.

                                 @param self <<in>> Cannot be NULL.
                                 @param i index of element to access, must be >= 0 and less than FooSeq_get_length
                                 @return a pointer to the i-th element
                                 @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.3.0/doc/api/connext_dds/api_c/group__DDSSequenceModule.html#">More Info</a>
                                */
                                UploadTime=WaterDispenserSeq_get_reference(&data_seq, j)->UploadTime;
                                Device_ID=WaterDispenserSeq_get_reference(&data_seq, j)->Device_ID;
                                TimeStamp=WaterDispenserSeq_get_reference(&data_seq, j)->TimeStamp;
                                Mac_Address=WaterDispenserSeq_get_reference(&data_seq, j)->Mac_Address;
                                SendTime=WaterDispenserSeq_get_reference(&data_seq, j)->SendTime;

                                Hot_Valve=WaterDispenserSeq_get_reference(&data_seq, j)->Hot_Valve;
                                Warm_Valve=WaterDispenserSeq_get_reference(&data_seq, j)->Warm_Valve;
                                Cold_Valve=WaterDispenserSeq_get_reference(&data_seq, j)->Cold_Valve;

                                Heating=WaterDispenserSeq_get_reference(&data_seq, j)->Heating;
                                Cooling=WaterDispenserSeq_get_reference(&data_seq, j)->Cooling;
                                Refilling=WaterDispenserSeq_get_reference(&data_seq, j)->Refilling;

                                Usage_L=WaterDispenserSeq_get_reference(&data_seq, j)->Usage_L;

                                /** This Function Print value of data type to standard out.
                                 @param a_data <<in>> String to be printed.
                                 @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.3.1/doc/api/connext_dds/api_cpp/classDDSStringTypeSupport.html#">More Info</a>
                                */
                                WaterDispenserTypeSupport_print_data(
                                        WaterDispenserSeq_get_reference(
                                                &data_seq,
                                                j));
                                /** This Function Returns the current value of the time.
                                 @param self <<in>> Cannot be NULL.
                                 @param current_time  <<inout>> Current time to be filled up. Cannot be NULL
                                 @return a pointer to the i-th element
                                 @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.3.0/doc/api/connext_dds/api_c/group__DDSDomainParticipantModule.html#">More Info</a>
                                */
                                DDS_DomainParticipant_get_current_time(participant, &current); 
                                long long tolong=current.sec;
                                long long delay=tolong*1000+current.nanosec/1000000-SendTime;
                                
                                water_doc = BCON_NEW(
                                            "UploadTime",
                                            BCON_UTF8(str_replace(UploadTime,"\r\n","")),
                                            "ReceiveTime",
                                            BCON_INT64(SendTime),
                                            
                                            "Hot_Valve",
                                            BCON_INT32(Hot_Valve),
                                            
                                            "Warm_Valve",
                                            BCON_INT32(Warm_Valve), 
                                            
                                            "Cold_Valve",
                                            BCON_INT32(Cold_Valve), 

                                            "Heating",
                                            BCON_INT32(Heating),

                                            "Cooling",
                                            BCON_INT32(Cooling), 

                                            "Refilling",
                                            BCON_INT32(Refilling),

                                            "Device_ID",
                                            BCON_UTF8(str_replace(Device_ID,"\r\n","")),


                                            "Usage_L",
                                            BCON_INT32(Usage_L),
                                            
                                            "TimeStamp",
                                            BCON_INT64(TimeStamp),
                                            
                                            "Mac_Address",
                                            BCON_UTF8(str_replace(Mac_Address,"\r\n","")),
                                            
                                            "delay",
                                            BCON_INT64(delay)
                                            );
                                if(TimeStamp!=0){  
                                    //if(delay<=10000){             
                                        if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, water_doc, NULL, &error)) {//插入文档
                                            fprintf (stderr, "%s\n", error.message);
                                        }
                                    //} 
                                }                
                            }
                           /** Indicates to the DDS_DataReader that the application is done accessing the collection of received_data and info_seq obtained by some earlier invocation of read or take on the DDS_DataReader.
                             @param received_data <<in>> user data type-specific FooSeq object where the received data samples was obtained from earlier invocation of read or take on the DDS_DataReader. Must be a valid non-NULL FooSeq. The function will fail with DDS_RETCODE_BAD_PARAMETER if it is NULL.
                             @param self  <<in>> Cannot be NULL.
                             @param info_seq  <<in>> a DDS_SampleInfoSeq object where the received sample info was obtained from earlier invocation of read or take on the DDS_DataReader. Must be a valid non-NULL DDS_SampleInfoSeq. The function will fail with DDS_RETCODE_BAD_PARAMETER if it is NULL.
                             @return One of the Standard Return Codes, DDS_RETCODE_PRECONDITION_NOT_MET or DDS_RETCODE_NOT_ENABLED.
                             @brief <a href="https://community.rti.com/static/documentation/connext-dds/5.3.0/doc/api/connext_dds/api_c/group__DDSReaderModule.html#">More Info</a>
                            */
                            WaterDispenserDataReader_return_loan( WaterDispenserReader, &data_seq, &info_seq);
                                   
                        }
                        
                    }
                }
            }

            
        
        //printf("WaterDispenser subscriber sleeping for %d sec...\n",poll_period.sec);

        //NDDS_Utility_sleep(&poll_period);
    }


    bson_destroy (insert);
    bson_destroy (&reply);
    bson_destroy (command);
    bson_free (str);
    /**
        * Release our handles and clean up libmongoc
        */
    mongoc_collection_destroy (collection);///释放表对象
    mongoc_database_destroy (database);///释放数据库对象
    mongoc_client_destroy (client);///释放连接对象
    mongoc_cleanup ();///释放libmongoc驱动 kc
    
    /** Delete all entities */
    retcode = DDS_WaitSet_delete(waitset);
    if (retcode != DDS_RETCODE_OK) {
        printf("delete waitset error %d\n", retcode);
    }

    /** Cleanup and delete all entities */
    return subscriber_shutdown(participant);
}

int main(int argc, char *argv[])
{
    int domain_id = 0;
    int sample_count = 0; /** infinite loop */

    if (argc >= 2) {
        domain_id = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /** Uncomment this to turn on additional logging
    NDDS_Config_Logger_set_verbosity_by_category(
        NDDS_Config_Logger_get_instance(),
        NDDS_CONFIG_LOG_CATEGORY_API, 
        NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */

    return subscriber_main(domain_id, sample_count);
}