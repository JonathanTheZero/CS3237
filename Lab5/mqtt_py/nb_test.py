from typing import Any
from nb_classifier import train_model, predict_window_state, WindowState


TESTING_DATA: list[tuple[float, float]] = [
    (25.0, 60.0),
    (30.0, 70.0),
    (15.5, 55.5),
    (35.0, 80.0),
    (20.0, 40.0),
    (27.0, 80.0),
    (27.0, 30.0),
    (27.0, 50.0),
    (13.0, 100.0),
    (33.0, 100.0),
]
ADDITIONAL_TESTING_DATA: list[tuple[Any, Any]] = [
    (22.0, 50.0),  # Average values
    (28.0, 65.0),  # Typical summer conditions
    (10.0, 30.0),  # Cold day with low humidity
    (0.0, 0.0),  # Extreme low conditions
    (40.0, 90.0),  # Hot and humid conditions
    (27.5, 55.0),  # Average comfortable range
    (32.0, 85.0),  # High temperature with high humidity
    (18.0, 45.0),  # Cool and dry conditions
    (29.0, 75.0),  # Warm and moderately humid
    (20.5, 100.0),  # Warm and fully saturated
    (1000000.0, 100000.0),
    (-234234234234, -3453452),
    # Invalid test cases
    ("thirty", "seventy"),  # Invalid string input
    (None, None),  # None values
    ([], []),  # Empty list
]


def test_model_with_values(
    testing_data: list[tuple[float, float]] = TESTING_DATA + ADDITIONAL_TESTING_DATA
) -> None:
    print("Testing model with custom input values...")

    for i, (temperature, humidity) in enumerate(testing_data):
        try:
            prediction: WindowState = predict_window_state(temperature, humidity)
            print(f"Test case {i+1}: Temperature={temperature}, Humidity={humidity}")
            print(f"Predicted window state: {prediction}")
        except Exception as e:
            print(f"Error predicting for test case {i+1}: {e}")


def value_test() -> None:
    train_model()
    print("-----------------------------------")
    test_model_with_values()


# step_size = How big the step increments should be, give as an percentage
def parameter_test(step_size: int = 2) -> None:
    accuracies: list[tuple[float, float]] = []

    for train_size in [
        round(x * step_size, 2)
        for x in range(int(1 / step_size), int(1 / step_size * 100) + 1)
    ]:
        accuracy: float | None = train_model(train_size=train_size)
        if accuracy is not None:
            accuracies.append((accuracy, train_size))

    top_accuracies: list[tuple[float, float]] = sorted(
        accuracies, key=lambda x: x[0], reverse=True
    )[:5]

    print("Top 5 accuracies:")
    for accuracy, train_size in top_accuracies:
        print(f"Train Size: {train_size}, Accuracy: {accuracy * 100:.2f}%")


if __name__ == "__main__":
    parameter_test()
