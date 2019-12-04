import artery.application.ItsG5Service;

simple VruDetectService like ItsG5Service
{
    double vruSendInterval = default(1s) @unit(s);  //the intervall between 2 vruDdetect messages
}
