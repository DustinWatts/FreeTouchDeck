// ------------------- class for debugging ------------------
class Debug {

public:
  const int LevelError = 0;
  const int LevelWarning = 1;
  const int LevelInfo = 2;
  
private:
  int m_loglevel = LevelInfo;
public:
  void SetLevel(int level)
  {
    m_loglevel = level;
  }
  void Error(const char* message)
  {
    if(m_loglevel >= LevelError){
    Serial.printf("[ERROR]: %s\n", message);
    }
  }
  void Warn(const char* message)
  {
    if(m_loglevel >= LevelWarning){
      Serial.printf("[WARNING]: %s\n", message);
    }
  }
  void Info(const char* message)
  {
    if(m_loglevel >= LevelInfo){
      Serial.printf("[INFO]: %s\n", message);
    }
  }
};
