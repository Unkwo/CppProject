#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <exception>
#include <iomanip>
using namespace std;

namespace DisasterCore{
    namespace Sensors{
        inline double computeHazardIndex(double rainfall, double slope){
            return rainfall * slope;
        }

        inline double computeHazardIndex(double moisture, int vibration){
            return moisture * vibration;
        }
    }

    namespace Alerts{
        int globalAlertLevel = 0;
    }
}

class TelemetryAuditor;

class SensorStation{
    string stationID;
    double hazard;

public:
    static int activeStations;

    SensorStation(string id = "", double h = 0): stationID(id), hazard(h){
        activeStations++;
    }

    SensorStation(const SensorStation& s){
        stationID = s.stationID;
        hazard = s.hazard;
        activeStations++;
    }

    ~SensorStation(){
        activeStations--;
    }

    friend class TelemetryAuditor;

    SensorStation operator +(const SensorStation& rhs){
        return SensorStation(
            stationID + "+" + rhs.stationID,
            hazard + rhs.hazard
        );
    }

    bool operator >(const SensorStation& rhs){
        return hazard > rhs.hazard;
    }

    friend ostream& operator<<(ostream& out, const SensorStation& s){
        out << s.stationID
            << " "
            << s.hazard;
        return out;
    }

    friend istream& operator>>(istream& in, SensorStation& s){
        in >> s.stationID >> s.hazard;
        return in;
    }

    double getHazard() const{
        return hazard;
    }

    static void showCount(){
        cout << "Active Stations = "
             << activeStations << endl;
    }
};

int SensorStation::activeStations = 0;

class TelemetryAuditor{
public:
    void inspect(const SensorStation& s){
        cout << "Auditor Access Granted\n";
    }
};

class TelemetryBuffer{
    int* buffer;
    int size;

public:
    TelemetryBuffer(int n = 5){
        size = n;
        buffer = new int[size];

        for(int i=0;i<size;i++)
            buffer[i]=0;
    }

    TelemetryBuffer(const TelemetryBuffer& t){
        size = t.size;
        buffer = new int[size];

        for(int i=0;i<size;i++)
            buffer[i] = t.buffer[i];
    }

    ~TelemetryBuffer(){
        delete[] buffer;
    }
};

class PowerSubsystem{
public:
    void status()
    {
        cout<<"Solar Power OK\n";
    }
};

class BaseSensorNode{
protected:
    PowerSubsystem power;

public:
    virtual void readTelemetry() = 0;
    virtual void evaluateThreshold() = 0;
    virtual ~BaseSensorNode(){}
};

class DisasterTelemetryDevice{
protected:
    int deviceID;

public:
    DisasterTelemetryDevice(int id=0) : deviceID(id){}
};

class SolarPoweredNode :virtual public DisasterTelemetryDevice{
public:
    SolarPoweredNode(int id=0): DisasterTelemetryDevice(id){}
};

class WirelessLoRaTransceiver :virtual public DisasterTelemetryDevice{
public:
    WirelessLoRaTransceiver(int id=0): DisasterTelemetryDevice(id){}
};

class AutonomousHybridProbe :
    public SolarPoweredNode,
    public WirelessLoRaTransceiver,
    public BaseSensorNode
{
public:
    AutonomousHybridProbe(int id)
        : DisasterTelemetryDevice(id),
          SolarPoweredNode(id),
          WirelessLoRaTransceiver(id)
    {}

    void readTelemetry() override{
        cout << "Reading telemetry...\n";
    }

    void evaluateThreshold() override{
        cout << "Threshold evaluated.\n";
    }
};

class SensorFailureException :
    public exception{
public:
    const char* what() const noexcept override
    {
        return "Sensor Failure";
    }
};

class TelemetryChecksumMismatch :
    public exception{
public:
    const char* what() const noexcept override
    {
        return "Checksum Mismatch";
    }
};

class ThresholdOverflowException :
    public exception
{
public:
    const char* what() const noexcept override
    {
        return "Threshold Overflow";
    }
};

class EvacuationPriority
{
public:
    bool operator()(const SensorStation& a,
                    const SensorStation& b)
    {
        return a.getHazard() >
               b.getHazard();
    }
};

ostream& alertFormat(ostream& os){
    os << fixed
       << setprecision(2)
       << setw(10);
    return os;
}

int main(){
    using namespace DisasterCore::Sensors;
    cout<<"Hazard Index = "
        << computeHazardIndex(12.5,7.2)
        << endl;

    SensorStation s1("HIM01",90);
    SensorStation s2("UTK05",70);
    SensorStation s3 = s1 + s2;
    cout<<s3<<endl;

    SensorStation::showCount();

    TelemetryAuditor ta;
    ta.inspect(s1);

    vector<BaseSensorNode*> nodes;
    nodes.push_back(new AutonomousHybridProbe(101));
    nodes.push_back(new AutonomousHybridProbe(102));

    for(auto n : nodes){
        n->readTelemetry();
        n->evaluateThreshold();
    }

    vector<SensorStation> region;
    region.push_back(s1);
    region.push_back(s2);
    region.push_back(s3);
    sort(region.begin(),
         region.end(),
         EvacuationPriority());
    for_each(region.begin(),
             region.end(),
             [](SensorStation s)
             {
                 cout<<s<<endl;
             });

    map<string,double> hazards;
    hazards["Uttarakhand"] = 95;
    hazards["Himachal"] = 85;
    fstream file(
        "disaster.dat",
        ios::binary |
        ios::out);

    if(file){
        long pos = file.tellp();
        int incidentID = 101;

        file.write(
            (char*)&incidentID,
            sizeof(incidentID));
        if(!file)
        {
            file.clear();
            file.seekp(pos);
        }
    }
    file.close();

    try{
        throw ThresholdOverflowException();
    }
    catch(exception& e){
        cout<<e.what()<<endl;
    }

    cout<<alertFormat
        << 123.456
        << endl;

    for(auto n : nodes)
        delete n;

    return 0;
}