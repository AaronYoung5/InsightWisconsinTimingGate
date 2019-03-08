import java.awt.Button;
import java.awt.Event;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Frame;
import java.awt.TextField;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.charset.StandardCharsets;

import javax.xml.soap.Text;

// Copy right: Mohammad SAFEEA
// 3rd-March-2018
// Java program to control the outputs:
// D0 to D8 of the ESP8266 through network

public class JavaFrame extends Frame implements ActionListener {

  Button[] b = new Button[14];
  String[] bCaption = new String[14];
  int fontSize = 32;

  Button bexit = new Button(" exit ");

  private TextField txt;
  private Button bConnect = new Button(" connect ");
  private TextField txtStatus;

  private Socket soc;

  private int fWidth = 0;
  private int fHeight = 0;

  JavaFrame() {
    System.out.println("Setting Up Java Frame");
    int w1, w2;

    this.setLayout(new FlowLayout());


    for (int i = 0; i < 7; i++) {
      bCaption[i] = "On_D" + Integer.toString(i);
      bCaption[i + 7] = "Off_D" + Integer.toString(i);
    }

    for (int i = 0; i < 7; i++) {
      b[i] = new Button(bCaption[i]);
      b[i + 7] = new Button(bCaption[i + 7]);
    }

    Font myFont = new Font("Courier", Font.ITALIC, fontSize);
    for (int i = 0; i < 14; i++) {
      b[i].setFont(myFont);
    }

    for (int i = 0; i < 7; i++) {
      this.add(b[i]);
      this.add(b[i + 7]);
    }

    bexit.setFont(myFont);
    this.add(bexit);

    txt = new TextField();
    txt.setColumns(15);
    txt.setText("192.168.4.1");
    txt.setFont(myFont);
    this.add(txt);


    txtStatus = new TextField();
    txtStatus.setColumns(15);
    txtStatus.setText("");
    txtStatus.setFont(myFont);
    this.add(txtStatus);

    bConnect.setFont(myFont);
    this.add(bConnect);

    fHeight = 0;
    for (int i = 0; i < 7; i++) {
      fHeight = fHeight + b[i].getHeight();
    }
    fHeight = fHeight + bexit.getHeight();
    fHeight = fHeight + txt.getHeight();
    fHeight = fHeight + bConnect.getHeight();
    int h1 = getInsets().top;
    double hScale = 1.0;
    fHeight = (int) Math.floor(fHeight * hScale) + h1;


    setTitle("ESP8266");
    this.resize(fWidth, fHeight);
    this.show();

    w1 = b[0].getWidth() + b[7].getWidth();
    w2 = txt.getWidth();
    double wScale = 1.1;
    if (w1 > w2) {
      fWidth = (int) Math.floor(w1 * wScale);
    } else {
      fWidth = (int) Math.floor(w2 * wScale);
    }

    fHeight = bConnect.getY() + bConnect.getHeight() +
            getInsets().top;

    this.resize(fWidth, fHeight);
    this.show();

    fHeight = bConnect.getY() + bConnect.getHeight() +
            getInsets().top;

    this.resize(fWidth, fHeight);
    this.show();
  }
  
  /**
   * Invoked when an action occurs.
   *
   * @param e the event to be processed
   */
  @Override
  public void actionPerformed(ActionEvent e) {
    System.out.println("Test");
  }

  public boolean action(Event e, Object ob) {
    System.out.println("Running Action");
    String caption = ob.toString();

    if (caption.contains("exit")) {
      closeConnecition();
      this.dispose();
      return true;
    }

    if (caption.contains("connect")) {
      // If the socket is already connected
      // then ignore the click
      System.out.println("Attempting To Connect");
      if (isConnected()) {
        System.out.println("Connected");
        return true;
      }

      try {
        soc = new Socket(txt.getText(), 1987);
        txtStatus.setText("Connected");
      } catch (UnknownHostException e1) {
        // TODO Auto-generated catch block
        e1.printStackTrace();
        txtStatus.setText("Can not connect");
      } catch (IOException e1) {
        // TODO Auto-generated catch block
        e1.printStackTrace();
        txtStatus.setText("Can not connect");

      }
      return true;
    }

    int aASCII = 97;
    int command = 0;
    for (int i = 0; i < 7; i++) {
      String bOn = "On_D" + Integer.toString(i);
      if (caption.contains(bOn)) {
        command = aASCII + i;
        // send command over network
        writeCommand(command);
        break;
      }
      String bOff = "Off_D" + Integer.toString(i);
      if (caption.contains(bOff)) {
        command = aASCII + i + 7;
        // send command over network
        writeCommand(command);
        break;
      }
    }

    return true;
  }

  private void closeConnecition() {
    System.out.println("Closing Connection");
    // If connected close the connection
    if (soc != null) {
      if (soc.isConnected()) {
        try {
          soc.close();
          txtStatus.setText("Socket closed");
        } catch (IOException e1) {
          // TODO Auto-generated catch block
          e1.printStackTrace();
          txtStatus.setText("Error closing socket");
        }
      }
    }

  }

  private boolean isConnected() {
    System.out.println("Checking Connection");
    // TCheck is socket is connected to server
    if (soc != null) {
      return soc.isConnected();
    }
    return false;
  }

  private void writeCommand(int command) {
    System.out.println("Writing Command");
    // If no socket is null return
    if (soc == null)
      return;
    // if socket is connected send command
    if (soc.isConnected()) {
      byte[] cmdByte = new byte[2];
      cmdByte[0] = (byte) command;
      byte[] temp;
      temp = "!".getBytes(StandardCharsets.US_ASCII);
      cmdByte[1] = temp[0];
      txtStatus.setText("Message sent");
      try {
        soc.getOutputStream().write(cmdByte);
        txtStatus.setText("Message sent");
      } catch (IOException e1) {
        // TODO Auto-generated catch block
        e1.printStackTrace();
        txtStatus.setText("Error");
      }
    }
  }


}
