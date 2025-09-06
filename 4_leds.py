import cv2
import mediapipe as mp
import requests
import time

# Replace with your ESP8266's actual IP address
ESP_IP = "http://10.1.47.20"  # Example: change to your ESP's IP

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

# Initialize camera
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    print("Error: Could not open camera")
    exit()

last_gesture_time = time.time()

def send_led_states(states):
    """
    states = [0/1, 0/1, 0/1, 0/1]  → Thumb, Index, Middle, Ring
    Example: [1,0,1,0] → Thumb ON, Index OFF, Middle ON, Ring OFF
    """
    try:
        url = f"{ESP_IP}/?t={states[0]}&i={states[1]}&m={states[2]}&r={states[3]}"
        print(f"Sending: {url}")  # Debug output
        response = requests.get(url, timeout=5.0)  # Increased timeout
        print(f"Response status: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"Network error: {e}")
        pass

def is_finger_up(landmarks, tip_id, pip_id):
    """Check if finger is extended by comparing tip and PIP joint positions"""
    return landmarks[tip_id].y < landmarks[pip_id].y

def is_thumb_up(landmarks, hand_label):
    """Check thumb state considering hand orientation (left/right)"""
    thumb_tip = landmarks[4]
    thumb_ip = landmarks[3]
    
    # For right hand: thumb up when tip.x > ip.x
    # For left hand: thumb up when tip.x < ip.x
    if hand_label == "Right":
        return thumb_tip.x < thumb_ip.x
    else:  # Left hand
        return thumb_tip.x > thumb_ip.x

with mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,  # Process only one hand for better performance
    min_detection_confidence=0.5, 
    min_tracking_confidence=0.5
) as hands:
    
    print("Hand gesture control started. Press ESC to exit.")
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Error: Failed to read from camera")
            break
        
        # Flip frame horizontally for mirror effect
        frame = cv2.flip(frame, 1)
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        
        # Process the frame
        results = hands.process(rgb_frame)
        
        # Initialize finger states (default OFF)
        states = [0, 0, 0, 0]  # Thumb, Index, Middle, Ring
        
        if results.multi_hand_landmarks and results.multi_handedness:
            for hand_landmarks, handedness in zip(results.multi_hand_landmarks, results.multi_handedness):
                # Draw hand landmarks
                mp_draw.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)
                
                # Get hand label (Left/Right)
                hand_label = handedness.classification[0].label
                landmarks = hand_landmarks.landmark
                
                # Check finger states
                # Thumb (special case due to different orientation)
                states[0] = 1 if is_thumb_up(landmarks, hand_label) else 0
                
                # Index finger (tip: 8, PIP: 6)
                states[1] = 1 if is_finger_up(landmarks, 8, 6) else 0
                
                # Middle finger (tip: 12, PIP: 10)
                states[2] = 1 if is_finger_up(landmarks, 12, 10) else 0
                
                # Ring finger (tip: 16, PIP: 14)
                states[3] = 1 if is_finger_up(landmarks, 16, 14) else 0
                
                # Display current finger states on frame
                cv2.putText(frame, f"T:{states[0]} I:{states[1]} M:{states[2]} R:{states[3]}", 
                           (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                
                # Update last gesture time when hand is detected
                last_gesture_time = time.time()
        
        # Send states to ESP when hand is detected
        if results.multi_hand_landmarks:
            send_led_states(states)
        
        # Check for timeout (no hand detected for 10 seconds)
        if time.time() - last_gesture_time > 10:
            print("No hand detected for 10 seconds. Exiting...")
            break
        
        # Display the frame
        cv2.imshow("Hand Gesture Control", frame)
        
        # Exit on ESC key
        if cv2.waitKey(1) & 0xFF == 27:  # ESC key
            print("ESC pressed. Exiting...")
            break

# Cleanup
cap.release()
cv2.destroyAllWindows()
print("Program terminated.")