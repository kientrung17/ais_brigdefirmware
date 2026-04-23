/*
 * File:   processormanager.h
 * Author: sangnv17@viettel.com.vn
 *
 * Created on December 1, 2019, 6:00 PM
 */

 #ifndef SERVICEMANAGER_H
 #define SERVICEMANAGER_H
 
 #include "taskabstract.h"
 #include <string>
 #include <map>

 /**
      * @brief The TaskManager class
      * This class for handleing message bus in the whole system
      */
 class TaskManager
 {
     public:
         static TaskManager* getInstance();
         ~TaskManager();
         /**
          * @brief registerProcessor
          * @param processor
          * @return
          * register processor to messages bus
          */
         bool registerProcessor(TaskAbstract* processor);
 
         /**
          * @brief deregisterProcessor
          * @param processorName
          * @return
          * deregister processor to messages bus
          */
         bool deregisterProcessor(std::string processorName);
         /**
          * @brief publish
          * @param msg
          * @param processorName
          * @return
          * send msg to specified processor
          */
        //  bool publish(std::shared_ptr<InnerMessage> msg, QString processorName);
        OSBase *getOSBase();

        void onTimer100Hz();
        
     private:
         TaskManager();
         static TaskManager self;
         OSBase *mOSBase;
         std::map<std::string, TaskAbstract*> mMapProcessor;
 };
 
 
 #endif // SERVICEMANAGER_H
 