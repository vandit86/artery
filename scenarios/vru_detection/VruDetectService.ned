import artery.application.ItsG5Service;

simple VruDetectService like ItsG5Service
{
    double vruSendInterval = default(1s) @unit(s);  //the intervall between 2 messages
    double keepInterval = default(1s) @unit(s);  //time to keep ped data on lists
    //double distance = default(50m) @unit(m);  //time to keep ped data on lists
}
